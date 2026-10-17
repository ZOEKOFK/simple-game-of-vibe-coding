// ============================================================
// game.cpp —— GameEngine 实现
// T1：窗口初始化、固定时间步主循环（accumulator 模式）、五状态空壳切换。
// T2：PLAYING 状态接入 八方向输入→Player 移动、摄像机跟随与竞技场绘制。
// T3：F1 调试刷怪 + 敌人追踪/分离 AI 更新与绘制、临调敌人数显示。
// T4：武器自动索敌/开火、子弹飞行与命中判定（命中暂只移除子弹）、命中次数显示。
// ============================================================
#include "game.h"

#include <algorithm>
#include <string>

#include "arena.h"
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

    // 数字键 1~5 切换状态：仅用于验收"五状态空壳"，后续任务替换为真实触发条件（保留）
    if (IsKeyPressed(KEY_ONE)) changeState(GameState::MAIN_MENU);
    if (IsKeyPressed(KEY_TWO)) changeState(GameState::PLAYING);
    if (IsKeyPressed(KEY_THREE)) changeState(GameState::PAUSED);
    if (IsKeyPressed(KEY_FOUR)) changeState(GameState::UPGRADE);
    if (IsKeyPressed(KEY_FIVE)) changeState(GameState::GAME_OVER);

    // T2：仅 PLAYING 状态响应移动（在固定步长内更新，逻辑与帧率解耦）
    if (state_ == GameState::PLAYING) {
        player_.update(dt, readMoveInput());

        // T3：F1 临时刷 10 只爬行者用于调试（正式波次逻辑 T8 接入）
        if (IsKeyPressed(KEY_F1)) {
            auto batch = spawner_.spawnWave(constants::enemy::kDebugSpawnCount, player_.pos());
            for (auto& e : batch) enemies_.push_back(std::move(e));
        }

        GameContext ctx{&enemies_, &projectiles_};

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

        // T4：子弹×敌人命中判定——朴素双循环（实体量大后的优化在 T11 换空间哈希）；
        //     命中暂只移除子弹并计数，伤害与击杀 T5 结算
        for (const auto& p : projectiles_) {
            if (!p->alive()) continue;
            for (const auto& e : enemies_) {
                if (!e) continue;
                const float rr = p->radius() + e->radius();
                const float dx = p->pos().x - e->pos().x;
                const float dy = p->pos().y - e->pos().y;
                if (dx * dx + dy * dy <= rr * rr) {
                    p->markDead();
                    ++hitCount_;
                    break;
                }
            }
        }

        // 移除已失效子弹（命中/超射程/出界）
        projectiles_.erase(
            std::remove_if(projectiles_.begin(), projectiles_.end(),
                           [](const auto& p) { return !p || !p->alive(); }),
            projectiles_.end());
    }
}

void GameEngine::draw() {
    // 第 1 步：把画面绘制到 1280x720 逻辑分辨率的渲染目标
    BeginTextureMode(renderTex_);
    {
        if (state_ == GameState::PLAYING) {
            // ---- T2：摄像机跟随 + 竞技场 + 玩家 ----
            ClearBackground(constants::arena::kOutsideColor);  // 场外底色（近黑）
            camera_.target = player_.pos();  // 镜头跟随：玩家恒保持屏幕中心
            BeginMode2D(camera_);
            {
                drawArena();                                  // 地面/网格/围墙（世界坐标）
                for (const auto& e : enemies_) e->draw();     // 敌人（世界坐标）
                for (const auto& p : projectiles_) p->draw(); // 子弹（黄色小圆）
                player_.draw();                               // 玩家（世界坐标）
            }
            EndMode2D();
            // 操作提示与临调显示（屏幕空间；正式 UI 由 T6 实现）
            DrawText("WASD / 方向键 移动    F1 刷怪    Esc 退出", 20, 20, 20, LIGHTGRAY);
            const std::string countText = "敌人数量: " + std::to_string(enemies_.size());
            DrawText(countText.c_str(), 20, 48, 20, GOLD);
            const std::string hitText = "命中次数: " + std::to_string(hitCount_);
            DrawText(hitText.c_str(), 20, 68, 20, YELLOW);
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

    UnloadRenderTexture(renderTex_);
    CloseWindow();
}

void GameEngine::resetGame() {
    // TODO(T10)：全量重置（开局/重开共用）
}
