// ============================================================
// game.cpp —— GameEngine 实现
// T1：窗口初始化、固定时间步主循环（accumulator 模式）、五状态空壳切换。
// T2：PLAYING 状态接入 八方向输入→Player 移动、摄像机跟随与竞技场绘制。
// T3：F1 调试刷怪 + 敌人追踪/分离 AI 更新与绘制、临调敌人数显示。
// T4：武器自动索敌/开火、子弹飞行与命中判定（命中暂只移除子弹）、命中次数显示。
// T5：伤害结算（命中扣血/击杀掉落经验点/接触受伤）、击退与无敌帧、HP 归零锁 0。
// T6：HUD（等级/经验条/血条/波次占位/敌人/击杀/武器说明）+ 中文字体加载。
// T7：经验吸取→升级三选一（UPGRADE 世界静止）→词条应用；Stats 全联动；每秒生命回复。
// ============================================================
#include "game.h"

#include <algorithm>
#include <string>

#include "arena.h"
#include "collision.h"  // T5：统一圆-圆检测
#include "constants.h"

namespace {

// 状态名对照表（空壳阶段用于屏幕提示，便于人工验收状态切换）
constexpr const char* kStateNames[] = {
    "主菜单 MAIN_MENU",
    "游戏进行中 PLAYING",
    "已暂停 PAUSED",
    "升级选择 UPGRADE",
    "游戏结束 GAME_OVER",
};

const char* stateName(GameEngine::GameState s) {
    return kStateNames[static_cast<int>(s)];
}

// 汇集八方向输入：WASD + 方向键（返回未归一化方向，归一化由 Player 负责）
Vector2 readMoveInput() {
    Vector2 dir{0.0f, 0.0f};
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dir.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dir.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dir.y += 1.0f;
    return dir;
}

}  // namespace

void GameEngine::changeState(GameState next) { state_ = next; }

void GameEngine::update(float dt) {
    // 【T1 验收要求】Esc 键退出窗口（保留）
    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
        return;
    }

    // 数字键 1~5 切换状态：仅用于验收"五状态空壳"，后续任务替换为真实触发条件（保留）。
    // T7 例外：UPGRADE 状态下 1/2/3 是选项快捷键，不参与状态切换
    if (state_ != GameState::UPGRADE) {
        if (IsKeyPressed(KEY_ONE)) changeState(GameState::MAIN_MENU);
        if (IsKeyPressed(KEY_TWO)) changeState(GameState::PLAYING);
        if (IsKeyPressed(KEY_THREE)) changeState(GameState::PAUSED);
        if (IsKeyPressed(KEY_FOUR)) changeState(GameState::UPGRADE);
        if (IsKeyPressed(KEY_FIVE)) changeState(GameState::GAME_OVER);
    }

    // T2：仅 PLAYING 状态响应移动（在固定步长内更新，逻辑与帧率解耦）
    if (state_ == GameState::PLAYING) {
        player_.update(dt, readMoveInput());
        ui_.update(dt, player_);  // T6：血条显示值平滑趋近实际 HP

        // T3：F1 临时刷 10 只爬行者用于调试（正式波次逻辑 T8 接入）
        if (IsKeyPressed(KEY_F1)) {
            auto batch = spawner_.spawnWave(constants::enemy::kDebugSpawnCount, player_.pos());
            for (auto& e : batch) enemies_.push_back(std::move(e));
        }

        GameContext ctx{&enemies_, &projectiles_, &player_, &xpOrbs_};

        // T3：敌人 AI 更新（追踪 + 分离 + 接触标记；伤害结算 T5）
        for (const auto& e : enemies_) {
            e->update(dt, player_.pos(), ctx);
        }

        // T4：武器自动索敌/开火（射程内最近敌人；无目标不开火）
        player_.updateWeapons(dt, ctx);

        // T4：子弹飞行（射程衰减/出界失效）
        for (const auto& p : projectiles_) {
            p->update(dt);
        }

        // T5：子弹×敌人命中结算——朴素双循环（实体量大后的优化在 T11 换空间哈希）；
        //     命中即单次伤害（穿透类武器后续扩展），击杀则掉落经验点（吸取 T7）
        for (const auto& p : projectiles_) {
            if (!p->alive()) continue;
            for (const auto& e : enemies_) {
                if (!e || e->isDead()) continue;
                if (collision::circlesOverlap(p->pos(), p->radius(),
                                              e->pos(), e->radius())) {
                    p->markDead();  // 命中子弹消失
                    ++hitCount_;
                    if (e->takeDamage(p->damage())) {
                        // 击杀：累计击杀数（T6 HUD）并掉落经验点（绿色小方块，吸取 T7）
                        ++killCount_;
                        xpOrbs_.emplace_back(e->pos(),
                                             static_cast<float>(e->xpValue()));
                    }
                    break;  // 单发子弹只结算一个敌人
                }
            }
        }

        // 移除已死亡敌人与已失效子弹（命中/超射程/出界）
        enemies_.erase(
            std::remove_if(enemies_.begin(), enemies_.end(),
                           [](const auto& e) { return !e || e->isDead(); }),
            enemies_.end());
        projectiles_.erase(
            std::remove_if(projectiles_.begin(), projectiles_.end(),
                           [](const auto& p) { return !p || !p->alive(); }),
            projectiles_.end());

        // T7：经验吸取——距玩家 ≤ 拾取半径的经验点自动吸收入账（升级溢出算法在 UpgradeSystem）
        const float pr = player_.pickupRadius();
        const float prSq = pr * pr;
        for (auto it = xpOrbs_.begin(); it != xpOrbs_.end();) {
            const float dx = it->pos().x - player_.pos().x;
            const float dy = it->pos().y - player_.pos().y;
            if (dx * dx + dy * dy <= prSq) {
                upgrade_.addExp(static_cast<int>(it->value()), ctx);
                it = xpOrbs_.erase(it);
            } else {
                ++it;
            }
        }

        // T7：生命回复持续效果——每秒 tick 结算一次（生命再生词条：+1 HP/s）
        regenTimer_ += dt;
        if (regenTimer_ >= 1.0f) {
            regenTimer_ -= 1.0f;
            if (player_.stats().hpRegen > 0.0f) {
                player_.heal(player_.stats().hpRegen);
            }
        }

        // T7：装备占位提示倒计时
        if (noticeTimer_ > 0.0f) noticeTimer_ -= dt;

        // T7：经验攒满升级请求 → 抽三选一并切入 UPGRADE（世界暂停静止）
        if (upgrade_.hasPendingUpgrades()) {
            choices_ = upgrade_.drawChoices(3, ctx);
            hoveredCard_ = -1;
            changeState(GameState::UPGRADE);
        }
    } else if (state_ == GameState::UPGRADE) {
        // T7：UPGRADE 状态不更新敌人/子弹/玩家（画面静止），仅响应三选一。
        //     鼠标判定：窗口坐标经 hudDest_/hudScale_ 逆变换到逻辑坐标，
        //     与 UIManager::upgradeCardRect 布局同一来源，保证零错位
        hoveredCard_ = -1;
        if (hudScale_ > 0.0f) {
            const Vector2 logic = toLogicCoords(GetMousePosition());
            for (int i = 0; i < static_cast<int>(choices_.size()) && i < 3; ++i) {
                if (CheckCollisionPointRec(logic, ui_.upgradeCardRect(i))) {
                    hoveredCard_ = i;
                    break;
                }
            }
        }
        int pick = -1;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoveredCard_ >= 0) {
            pick = hoveredCard_;
        } else if (IsKeyPressed(KEY_ONE)) {
            pick = 0;
        } else if (IsKeyPressed(KEY_TWO)) {
            pick = 1;
        } else if (IsKeyPressed(KEY_THREE)) {
            pick = 2;
        }
        if (pick >= 0 && pick < static_cast<int>(choices_.size())) {
            resolveUpgradeChoice(pick);
        }
    }
}

void GameEngine::resolveUpgradeChoice(int idx) {
    if (idx < 0 || idx >= static_cast<int>(choices_.size())) return;
    const Affix chosen = choices_[idx];

    GameContext ctx{&enemies_, &projectiles_, &player_, &xpOrbs_};
    upgrade_.applyChoice(chosen, ctx);  // 属性类 → 修改 Stats；装备类 → 占位补偿属性

    // 装备类占位提示：选择后屏幕中央显示 3 秒（字体不可用回退英文）
    if (!chosen.equipNotice.empty()) {
        noticeText_ = ui_.cnFontAvailable() ? chosen.equipNotice : chosen.equipNoticeEn;
        noticeTimer_ = 3.0f;
    }

    // 连升多级：消耗一次升级请求，仍有剩余则留在 UPGRADE 重新三选一
    if (upgrade_.consumePendingUpgrade()) {
        choices_ = upgrade_.drawChoices(3, ctx);
        hoveredCard_ = -1;
    } else {
        changeState(GameState::PLAYING);
    }
}

Vector2 GameEngine::toLogicCoords(Vector2 winPos) const {
    // 窗口坐标 → 逻辑坐标：减去画面原点偏移后除以等比缩放系数
    return Vector2{(winPos.x - hudDest_.x) / hudScale_,
                   (winPos.y - hudDest_.y) / hudScale_};
}

void GameEngine::draw() {
    // 第 1 步：把画面绘制到 1280x720 逻辑分辨率的渲染目标
    BeginTextureMode(renderTex_);
    {
        if (state_ == GameState::PLAYING || state_ == GameState::UPGRADE) {
            // ---- T2：摄像机跟随 + 竞技场 + 玩家 ----
            // T7：UPGRADE 状态下世界照常绘制但不更新（敌我静止，画面定格）
            ClearBackground(constants::arena::kOutsideColor);  // 场外底色（近黑）
            camera_.target = player_.pos();  // 镜头跟随：玩家恒保持屏幕中心
            BeginMode2D(camera_);
            {
                drawArena();                                  // 地面/网格/围墙（世界坐标）
                for (const auto& e : enemies_) e->draw();     // 敌人（世界坐标）
                for (const auto& p : projectiles_) p->draw(); // 子弹（黄色小圆）
                for (const auto& o : xpOrbs_) o.draw();       // 经验点（绿色小方块，T5）
                player_.draw();                               // 玩家（世界坐标）
            }
            EndMode2D();
            // ---- T6 HUD（屏幕空间，替换 T4/T5 左上角临时调试文字）----
            HudSnapshot snap;
            snap.player = &player_;
            snap.enemyCount = static_cast<int>(enemies_.size());
            snap.wave = wave_;
            snap.waveTimer = waveTimer_;
            snap.killCount = killCount_;
            snap.level = upgrade_.level();                    // T7：等级/经验改由 UpgradeSystem 提供
            snap.exp = upgrade_.expInLevel();
            snap.expToNext = static_cast<float>(upgrade_.expToNext());
            ui_.drawHUD(snap);

            // 操作提示：屏幕左下角（T6 起改走中文字体，避免默认字体乱码）
            ui_.drawText("WASD / 方向键 移动     F1 刷怪     Esc 退出",
                         20.0f, constants::window::kLogicHeight - 36.0f,
                         16, LIGHTGRAY);
            // T7：装备占位提示（选择到装备类词条后屏幕中央显示 3 秒）
            if (noticeTimer_ > 0.0f) {
                ui_.drawCenterNotice(noticeText_.c_str(), 120.0f, 20, WHITE);
            }
            // T5 临时处理：HP 归零仅锁定为 0 并中央提示，正式死亡/结算流程 T10 实现
            if (player_.hp() <= 0) {
                // 字体不可用时回退英文文案（默认字体无 CJK 字形，防乱码）
                ui_.drawCenterNotice(ui_.cnFontAvailable()
                                         ? "（死亡逻辑将在后续任务实现）"
                                         : "(Death logic to be implemented in later tasks)",
                                     0.0f, 28, RED);
            }
            // T7：UPGRADE 遮罩压暗静止世界 + 三选一卡片
            if (state_ == GameState::UPGRADE) {
                DrawRectangle(0, 0, constants::window::kLogicWidth,
                              constants::window::kLogicHeight,
                              constants::ui::kDimOverlay);
                ui_.drawUpgradeChoices(choices_, hoveredCard_);
            }
        } else {
            // ---- T1 空壳展示（保留）：黑底 + 当前状态名 ----
            ClearBackground(BLACK);  // 验收预期：黑色窗口
            DrawText(stateName(state_), 40, 40, 28, WHITE);
            DrawText("Esc 退出 | 数字键 1~5 切换状态", 40, 80, 20, LIGHTGRAY);
        }
    }
    EndTextureMode();

    // 第 2 步：把渲染目标等比缩放到窗口（保持 16:9，多余部分黑边）
    BeginDrawing();
    {
        ClearBackground(BLACK);
        const float winW = static_cast<float>(GetScreenWidth());
        const float winH = static_cast<float>(GetScreenHeight());
        // 取宽、高两个方向缩放系数中较小者，保证画面完整且不变形
        const float scale = (winW / constants::window::kLogicWidth <
                             winH / constants::window::kLogicHeight)
                                ? (winW / constants::window::kLogicWidth)
                                : (winH / constants::window::kLogicHeight);
        const Rectangle dest{
            (winW - constants::window::kLogicWidth * scale) * 0.5f,
            (winH - constants::window::kLogicHeight * scale) * 0.5f,
            constants::window::kLogicWidth * scale,
            constants::window::kLogicHeight * scale,
        };
        // T7：保存画面映射（原点偏移 + 缩放），供 update 中鼠标坐标逆变换判定卡片
        hudDest_ = dest;
        hudScale_ = scale;
        // 渲染纹理 Y 轴向上（负高），修正 DrawTexturePro 的方向
        const Rectangle src{
            0.0f, 0.0f,
            static_cast<float>(constants::window::kLogicWidth),
            -static_cast<float>(constants::window::kLogicHeight),
        };
        DrawTexturePro(renderTex_.texture, src, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    }
    EndDrawing();
}

void GameEngine::run() {
    // raylib 要求配置标志在 InitWindow 之前设置
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // 窗口可缩放，画面由 draw() 等比缩放
    InitWindow(constants::window::kLogicWidth, constants::window::kLogicHeight,
               constants::window::kTitle);
    SetExitKey(KEY_NULL);  // 关闭 raylib 默认 Esc 退出，改由 update() 统一处理
    SetTargetFPS(constants::window::kTargetFps);

    renderTex_ = LoadRenderTexture(constants::window::kLogicWidth,
                                   constants::window::kLogicHeight);
    ui_.load();  // T6：HUD 中文字体（LoadFontEx 需要 GL 上下文，必须在 InitWindow 后）

    // T2：初始化 2D 摄像机——offset 为逻辑分辨率中心、target 对准玩家出生点，
    //     使玩家始终位于屏幕正中（target 每帧在 draw() 中跟随玩家）。
    camera_ = Camera2D{{constants::window::kLogicWidth * 0.5f,
                        constants::window::kLogicHeight * 0.5f},
                       {constants::player::kStartX, constants::player::kStartY},
                       0.0f, 1.0f};

    // ---- 固定时间步长主循环（accumulator 模式）----
    // 逻辑更新恒以 1/60s 步进，与显示帧率解耦，保证不同帧率下逻辑一致。
    float prevTime = static_cast<float>(GetTime());
    while (!WindowShouldClose()) {
        const float now = static_cast<float>(GetTime());
        float frameTime = now - prevTime;
        prevTime = now;
        if (frameTime > constants::loop::kMaxFrameTime) {
            frameTime = constants::loop::kMaxFrameTime;  // 卡顿保护，防"死亡螺旋"
        }
        accumulator_ += frameTime;
        while (accumulator_ >= constants::loop::kFixedDt) {
            update(constants::loop::kFixedDt);
            accumulator_ -= constants::loop::kFixedDt;
        }
        draw();
    }

    ui_.unload();  // T6：窗口关闭前释放字体资源（避免 GL 上下文失效后析构报错）
    UnloadRenderTexture(renderTex_);
    CloseWindow();
}

void GameEngine::resetGame() {
    // TODO(T10)：全量重置（开局/重开共用）
}
