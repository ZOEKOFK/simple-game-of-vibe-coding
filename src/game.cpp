// game.cpp —— GameEngine 实现
// T1：窗口初始化、固定时间步主循环（accumulator 模式）、五状态空壳切换。
// T2：PLAYING 状态接入 八方向输入→Player 移动、摄像机跟随与竞技场绘制。
// T3：F1 调试刷怪 + 敌人追踪/分离 AI 更新与绘制、临调敌人数显示。
// T4：武器自动索敌/开火、子弹飞行与命中判定（命中暂只移除子弹）、命中次数显示。
// T5：伤害结算（命中扣血/击杀掉落经验点/接触受伤）、击退与无敌帧、HP 归零锁 0。
// T6：HUD（等级/经验条/血条/波次占位/敌人/击杀/武器说明）+ 中文字体加载。
// T7：经验吸取→升级三选一（UPGRADE 世界静止）→词条应用；Stats 全联动；每秒生命回复。
// T8：波次系统（WaveManager 状态机接管刷怪）+ 喷吐者/蛮牛差异化 AI + 敌弹伤玩家。
// T10：完整流程闭环——主菜单开局（Enter/点击）/Esc 暂停/HP 归零结算/重开重置；
//      resetGame 统一重置；数字键 1~5 状态切换与 Esc 退出（T1 空壳）正式移除。
// T11：性能与体验——空间哈希网格碰撞（docs/03 2.8）、受击闪白/命中粒子/击杀飘字/
//      屏幕震动/死亡定格、程序生成音效、性能日志（perf_log.txt）与 F2 压力刷怪。
// ============================================================
#include "game.h"

#include <algorithm>
#include <cstdio>   // T11：性能日志落盘
#include <string>

#include "arena.h"
#include "collision.h"  // T5：统一圆-圆检测；T11：空间哈希网格
#include "constants.h"

namespace {

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
    // 【T10 流程改造】Esc 不再退出窗口：PLAYING/PAUSED 间切换暂停；
    // 退出走 主菜单/暂停菜单 的"退出游戏"项（关闭窗口在 run() 主循环安全收尾）。
    // T1 空壳的"数字键 1~5 切换状态"调试行为正式移除（真实流程已接管）。

    if (state_ == GameState::MAIN_MENU) {
        // 开始游戏 → 全量重置开新局；退出游戏 → 请求关闭窗口
        const int sel = handleMenuInput(2);
        if (sel == 0) {
            resetGame();
            changeState(GameState::PLAYING);
        } else if (sel == 1) {
            exitRequested_ = true;
        }
        return;
    }

    if (state_ == GameState::PAUSED) {
        // Esc 继续；或菜单选择：继续 / 返回主菜单 / 退出
        if (IsKeyPressed(KEY_ESCAPE)) {
            changeState(GameState::PLAYING);
            return;
        }
        const int sel = handleMenuInput(3);
        if (sel == 0) {
            changeState(GameState::PLAYING);
        } else if (sel == 1) {
            menuCursor_ = 0;  // 回主菜单：光标回到"开始游戏"
            changeState(GameState::MAIN_MENU);
        } else if (sel == 2) {
            exitRequested_ = true;
        }
        return;
    }

    if (state_ == GameState::GAME_OVER) {
        // 重新开始 → 直接开新局；返回主菜单
        const int sel = handleMenuInput(2);
        if (sel == 0) {
            resetGame();
            changeState(GameState::PLAYING);
        } else if (sel == 1) {
            menuCursor_ = 0;  // 回主菜单：光标回到"开始游戏"
            changeState(GameState::MAIN_MENU);
        }
        return;
    }

    // T2：仅 PLAYING 状态响应移动（在固定步长内更新，逻辑与帧率解耦）
    if (state_ == GameState::PLAYING) {
        // Esc 切换暂停（画面冻结由 PAUSED 分支 + draw 压暗实现）
        if (IsKeyPressed(KEY_ESCAPE)) {
            menuCursor_ = 0;  // 进入暂停菜单：光标默认停在"继续游戏"
            changeState(GameState::PAUSED);
            return;
        }

        // T10：存活时长累计（结算界面展示）
        survivalTime_ += dt;

        // T11：死亡定格——HP≤0 先冻结世界 kDeathFreezeTime（死亡震动可见）再进结算；
        //      定格期间不再移动/开火/结算，仅递减计时（死亡优先于升级，与 T10 判定一致）
        if (player_.hp() <= 0) {
            if (deathTimer_ > 0.0f) {
                deathTimer_ -= dt;
                if (deathTimer_ <= 0.0f) {
                    // 定格结束：汇总本局统计并切 GAME_OVER
                    endStats_.wavesSurvived = waveManager_.wave() - 1;  // 已完成波数 = 当前波号-1
                    endStats_.killCount = killCount_;
                    endStats_.level = upgrade_.level();
                    endStats_.elapsedSec = survivalTime_;
                    menuCursor_ = 0;
                    changeState(GameState::GAME_OVER);
                }
            } else {
                // 首次检测死亡：触发死亡震动 + 音效，进入定格
                deathTimer_ = constants::fx::kDeathFreezeTime;
                effects_.addShake(constants::fx::kShakeAmpDeath,
                                  constants::fx::kShakeDurDeath);
                audio_.play(SoundBank::Kind::Death);
            }
            return;
        }

        // T11：玩家受击震动判定基线——帧首记录 HP，伤害结算后比较下降即受击
        const int hpBeforeDamage = player_.hp();
        perfCollisionChecks_ = 0;  // T11：本帧碰撞比较计数清零

        player_.update(dt, readMoveInput());
        ui_.update(dt, player_);  // T6：血条显示值平滑趋近实际 HP

        // T3：F1 调试刷 10 只爬行者（T8 起为开发调试用快捷键，正式波次由 WaveManager 自动驱动）
        if (IsKeyPressed(KEY_F1)) {
            auto batch = spawner_.spawnWave(constants::enemy::kDebugSpawnCount, player_.pos());
            for (auto& e : batch) enemies_.push_back(std::move(e));
        }
        // T11：F2 性能压力刷 200 只（同屏 200 敌人帧率对比验证；调试/报告素材用）
        if (IsKeyPressed(KEY_F2)) {
            auto batch = spawner_.spawnWave(constants::enemy::kStressSpawnCount, player_.pos());
            for (auto& e : batch) enemies_.push_back(std::move(e));
        }

        // T11：性能日志——本固定步更新耗时起点（覆盖 PLAYING 全流程，含碰撞/吸取）
        const double stepStart = GetTime();
        GameContext ctx{&enemies_, &projectiles_, &player_, &xpOrbs_, &spawner_, &audio_};

        // T8：波次状态机（WAITING 3s 倒计时 → SPAWNING 分批投放 → FIGHTING 清场判下一波）
        waveManager_.update(dt, ctx);

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

        // T11：空间哈希网格重建（子弹移动后：登记敌人/子弹/经验点当前帧位置，
        //      碰撞与吸取查询共享；朴素双循环已替换——精确相交仍走
        //      circlesOverlap / 距离检查，仅候选范围缩小到邻域，行为等价）
        grid_.clear();
        for (const auto& e : enemies_) grid_.addEnemy(e.get());
        for (const auto& p : projectiles_) grid_.addBullet(p.get());
        for (auto& o : xpOrbs_) grid_.addOrb(&o);

        // T5/T8/T9：子弹×敌人命中结算——空间哈希邻域查询（docs/03 2.8 落地替换朴素双循环）；
        //     仅玩家阵营弹参与敌人命中（T8：喷吐者敌弹走下方"敌弹×玩家"结算，不伤同胞）；
        //     T9 穿透：穿甲弹可穿过若干敌人，同一子弹对同一敌人只结算一次（hasHit 判重）。
        //     死亡统一在帧末移除扫描结算（击杀数/波次账本/经验掉落），保证飞刃等多种死因一致。
        std::vector<Enemy*> hitCandidates;  // T11：邻域候选缓冲（复用避免重复分配）
        for (const auto& p : projectiles_) {
            if (!p->alive()) continue;
            if (p->faction() != Projectile::Faction::PlayerSide) continue;  // T8：敌弹跳过
            hitCandidates.clear();
            grid_.queryEnemies(p->pos(), p->radius(), hitCandidates);
            perfCollisionChecks_ += static_cast<int>(hitCandidates.size());
            for (auto* e : hitCandidates) {
                if (!e || e->isDead()) continue;
                if (p->hasHit(e)) continue;  // T9 穿透：同弹同敌不重复结算
                if (collision::circlesOverlap(p->pos(), p->radius(),
                                              e->pos(), e->radius())) {
                    ++hitCount_;
                    e->takeDamage(p->damage());
                    p->recordHit(e);  // T9：命中记录（穿透判定用）
                    // T9 吸血：按本次结算伤害比例回血（飞刃吸血在其武器层结算）
                    if (player_.stats().lifestealPct > 0.0f) {
                        player_.heal(p->damage() * player_.stats().lifestealPct);
                    }
                    // T11 打击感：命中火花粒子 + 命中音效（限流在 SoundBank 内）
                    effects_.spawnBurst(e->pos(), constants::fx::kHitParticleCount,
                                        constants::fx::kSparkColor,
                                        constants::fx::kParticleSpeed,
                                        constants::fx::kParticleLife);
                    audio_.play(SoundBank::Kind::Hit);
                    if (p->pierces() > 0) {
                        p->consumePierce();  // T9：穿甲弹——消耗一次穿透继续飞行
                    } else {
                        p->markDead();       // 无穿透：命中即消失（T8 原行为）
                    }
                    break;  // 单发子弹每帧至多结算一个敌人
                }
            }
        }

        // T8：敌方弹体 × 玩家命中结算（喷吐者弹：EnemySide 阵营；命中玩家直接结算伤害）。
        //     玩家侧无敌帧与击退在 Player::takeDamage 内统一处理——弹速 180 可走位躲开
        // T11：改为空间哈希邻域查询；命中玩家时附带红色受击粒子
        std::vector<Projectile*> playerHitCandidates;
        grid_.queryBullets(player_.pos(), player_.radius(), playerHitCandidates);
        perfCollisionChecks_ += static_cast<int>(playerHitCandidates.size());
        for (auto* p : playerHitCandidates) {
            if (!p->alive() || p->faction() != Projectile::Faction::EnemySide) continue;
            if (collision::circlesOverlap(p->pos(), p->radius(),
                                          player_.pos(), player_.radius())) {
                player_.takeDamage(p->damage(), p->pos());
                p->markDead();
                effects_.spawnBurst(player_.pos(), 6,
                                   Color{255, 96, 72, 255},  // 红：玩家受击反馈
                                   constants::fx::kParticleSpeed,
                                   constants::fx::kParticleLife);
            }
        }

        // T9：死亡统一结算——所有死因（子弹/环绕飞刃/后续武器）的击杀数、波次账本、
        //     经验掉落集中在此处理，保证任意武器击杀的行为一致（对 T8 行为等价）
        // T11：击杀打击感——爆散粒子 + 击杀数字飘字 + 击杀音效；精英（蛮牛）额外震动
        for (const auto& e : enemies_) {
            if (e && e->isDead()) {
                ++killCount_;
                waveManager_.notifyEnemyDied();  // T8：本波存活账本递减（清场判定）
                xpOrbs_.emplace_back(e->pos(), static_cast<float>(e->xpValue()));
                effects_.spawnBurst(e->pos(), constants::fx::kKillParticleCount,
                                    constants::fx::kKillColor,
                                    constants::fx::kParticleSpeed * 1.3f,
                                    constants::fx::kParticleLife * 1.4f);
                effects_.spawnKillText(e->pos(),
                                       "+" + std::to_string(e->xpValue()),
                                       constants::fx::kKillColor);
                audio_.play(SoundBank::Kind::Kill);
                if (e->isElite()) {
                    effects_.addShake(constants::fx::kShakeAmpBrute,
                                      constants::fx::kShakeDurBrute);
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

        // T11：打击感表现更新（粒子/飘字寿命推进；震动为时间驱动，此处无需处理）
        effects_.update(dt);

        // T11：玩家受击震动——帧首记录 HP，此处比较下降即本帧受击（含接触/敌弹）
        if (player_.hp() < hpBeforeDamage) {
            effects_.addShake(constants::fx::kShakeAmpHurt,
                              constants::fx::kShakeDurHurt);
        }

        // T7：经验吸取——距玩家 ≤ 拾取半径的经验点自动吸收入账（升级溢出算法在 UpgradeSystem）
        // T11：改为空间哈希邻域查询（候选过滤仍用距离平方判定，行为等价）
        const float pr = player_.pickupRadius();
        const float prSq = pr * pr;
        std::vector<XPOrb*> absorbed;  // 待移除经验点（循环后统一擦除，保持候选迭代器有效）
        std::vector<XPOrb*> orbCandidates;
        grid_.queryOrbs(player_.pos(), pr, orbCandidates);
        perfCollisionChecks_ += static_cast<int>(orbCandidates.size());
        for (auto* o : orbCandidates) {
            const float dx = o->pos().x - player_.pos().x;
            const float dy = o->pos().y - player_.pos().y;
            if (dx * dx + dy * dy <= prSq) {
                upgrade_.addExp(static_cast<int>(o->value()), ctx);
                absorbed.push_back(o);
            }
        }
        if (!absorbed.empty()) {
            xpOrbs_.erase(
                std::remove_if(xpOrbs_.begin(), xpOrbs_.end(),
                               [&](const XPOrb& orb) {
                                   return std::find(absorbed.begin(), absorbed.end(),
                                                    &orb) != absorbed.end();
                               }),
                xpOrbs_.end());
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

        // T11：性能日志累计（PLAYING 每固定步；敌人数 ≥ 阈值时每秒写一行 perf_log.txt）
        perfTimer_ += dt;
        ++perfSteps_;
        perfMsSum_ += static_cast<float>(GetTime() - stepStart) * 1000.0f;
        perfChecksSum_ += perfCollisionChecks_;
        perfCandSum_ += grid_.candidates();
        if (perfTimer_ >= 1.0f) {
            perfTimer_ -= 1.0f;
            if (static_cast<int>(enemies_.size()) >= constants::fx::kPerfLogEnemyThreshold) {
                logPerf();  // 敌人数达标才落盘（正常战斗低频，压力测试高频）
            } else {
                perfSteps_ = 0;
                perfMsSum_ = 0.0f;
                perfChecksSum_ = 0;
                perfCandSum_ = 0;
            }
        }

        // T10：HP 归零 → 死亡定格已在上方接管（此判定移除）；
        //     升级请求 → 抽三选一并切入 UPGRADE（世界暂停静止）
        if (upgrade_.hasPendingUpgrades()) {
            // T7：经验攒满升级请求 → 抽三选一并切入 UPGRADE（世界暂停静止）
            choices_ = upgrade_.drawChoices(3, ctx);
            hoveredCard_ = -1;
            audio_.play(SoundBank::Kind::Upgrade);  // T11：升级音效（进入三选一即播）
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

    GameContext ctx{&enemies_, &projectiles_, &player_, &xpOrbs_, &spawner_, &audio_};
    // 属性类 → 修改 Stats；装备类 → 真实装备/升级（T9），返回 1 新获得 / 2 已升级 / 0 其余
    const int result = upgrade_.applyChoice(chosen, ctx);

    // 装备类提示（T9）：按返回分支显示"已获取「XX」/ 已升级「XX」" 3 秒（字体不可用回退英文）
    if (result > 0) {
        if (ui_.cnFontAvailable()) {
            noticeText_ = (result == 1 ? "已获取「" : "已升级「") + chosen.name + "」";
        } else {
            noticeText_ = chosen.nameEn + (result == 1 ? " acquired" : " upgraded");
        }
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
        // T10：世界+世界态（PLAYING 实时 / UPGRADE 定格 / PAUSED 冻结）共用绘制路径
        if (state_ == GameState::PLAYING || state_ == GameState::UPGRADE ||
            state_ == GameState::PAUSED) {
            // ---- T2：摄像机跟随 + 竞技场 + 玩家 ----
            // T7：UPGRADE 状态下世界照常绘制但不更新（敌我静止，画面定格）
            // T10：PAUSED 同样只绘制不更新（冻结画面，叠压暗 + 暂停菜单）
            ClearBackground(constants::arena::kOutsideColor);  // 场外底色（近黑）
            camera_.target = player_.pos();  // 镜头跟随：玩家恒保持屏幕中心
            // T11：屏幕轻微震动——摄像机偏移叠加衰减随机抖动（无震动时为 0）
            const Vector2 shake = effects_.shakeOffset();
            camera_.offset = Vector2{constants::window::kLogicWidth * 0.5f + shake.x,
                                     constants::window::kLogicHeight * 0.5f + shake.y};
            BeginMode2D(camera_);
            {
                drawArena();                                  // 地面/网格/围墙（世界坐标）
                for (const auto& e : enemies_) {              // 敌人（世界坐标）
                    e->draw();
                    // T11：受击闪白——白色半透明层叠加（命中瞬间全亮，0.1s 内渐隐）
                    const float fa = e->hitFlashAlpha();
                    if (fa > 0.0f) {
                        DrawCircleV(e->pos(), e->radius(),
                                    Color{255, 255, 255,
                                          static_cast<unsigned char>(140.0f * fa + 20.0f)});
                    }
                }
                for (const auto& p : projectiles_) p->draw(); // 子弹（黄色小圆）
                for (const auto& o : xpOrbs_) o.draw();       // 经验点（绿色小方块，T5）
                player_.draw();                               // 玩家（世界坐标）
                player_.drawWeapons();                        // T9：环绕飞刃等持续武器自绘
                effects_.draw();  // T11：粒子与击杀飘字（世界坐标，绘制在实体之上）
            }
            EndMode2D();
            // ---- T6 HUD（屏幕空间，替换 T4/T5 左上角临时调试文字）----
            HudSnapshot snap;
            snap.player = &player_;
            snap.enemyCount = static_cast<int>(enemies_.size());
            snap.wave = waveManager_.wave();               // T8：当前波号（WAITING 即下一波）
            snap.waveState = static_cast<int>(waveManager_.state());  // 0 倒计时 / 1 刷怪 / 2 战斗
            snap.restTimeLeft = waveManager_.restTimeLeft();
            snap.remainingToSpawn = waveManager_.remainingToSpawn();
            snap.aliveInWave = waveManager_.aliveLeft();
            snap.killCount = killCount_;
            snap.level = upgrade_.level();                    // T7：等级/经验改由 UpgradeSystem 提供
            snap.exp = upgrade_.expInLevel();
            snap.expToNext = static_cast<float>(upgrade_.expToNext());
            ui_.drawHUD(snap);

            // 操作提示：屏幕左下角（T6 起改走中文字体，避免默认字体乱码）
            // T10：Esc 语义由"退出"改为"暂停"（退出走菜单项）
            // T11：F2 性能压力刷怪（调试/报告素材用）
            ui_.drawText("WASD / 方向键 移动     F1 调试刷怪    F2 压力刷怪    Esc 暂停",
                         20.0f, constants::window::kLogicHeight - 36.0f,
                         16, LIGHTGRAY);
            // T7：装备占位提示（选择到装备类词条后屏幕中央显示 3 秒）
            if (noticeTimer_ > 0.0f) {
                ui_.drawCenterNotice(noticeText_.c_str(), 120.0f, 20, WHITE);
            }
            // T10：PAUSED 压暗冻结画面 + 暂停菜单（继续/返回主菜单/退出）
            if (state_ == GameState::PAUSED) {
                DrawRectangle(0, 0, constants::window::kLogicWidth,
                              constants::window::kLogicHeight,
                              constants::ui::kDimOverlay);
                ui_.drawPauseMenu(menuCursor_);
            }
            // T7：UPGRADE 遮罩压暗静止世界 + 三选一卡片
            if (state_ == GameState::UPGRADE) {
                DrawRectangle(0, 0, constants::window::kLogicWidth,
                              constants::window::kLogicHeight,
                              constants::ui::kDimOverlay);
                ui_.drawUpgradeChoices(choices_, hoveredCard_, &player_);
            }
        } else {
            // ---- T10：主菜单 / 死亡结算（黑底整屏界面）----
            ClearBackground(BLACK);
            if (state_ == GameState::MAIN_MENU) {
                ui_.drawMainMenu(menuCursor_);
            } else {  // GAME_OVER：本局统计 + 重新开始/返回主菜单
                ui_.drawGameOverScreen(endStats_, menuCursor_);
            }
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

    // T11：音频设备与程序生成音效（InitAudioDevice 在 InitWindow 之后；失败静默降级）
    InitAudioDevice();
    if (IsAudioDeviceReady()) {
        audio_.init();
        SetMasterVolume(0.5f);  // 总体音量压半，避免合成音效突兀
    }

    // T2：初始化 2D 摄像机——offset 为逻辑分辨率中心、target 对准玩家出生点，
    //     使玩家始终位于屏幕正中（target 每帧在 draw() 中跟随玩家）。
    camera_ = Camera2D{{constants::window::kLogicWidth * 0.5f,
                        constants::window::kLogicHeight * 0.5f},
                       {constants::player::kStartX, constants::player::kStartY},
                       0.0f, 1.0f};

    // ---- 固定时间步长主循环（accumulator 模式）----
    // 逻辑更新恒以 1/60s 步进，与显示帧率解耦，保证不同帧率下逻辑一致。
    // T10：菜单"退出游戏"置 exitRequested_，主循环在此收尾（此时状态仍可安全绘制）
    float prevTime = static_cast<float>(GetTime());
    while (!WindowShouldClose() && !exitRequested_) {
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
    audio_.unload();      // T11：释放程序生成音效资源（CloseAudioDevice 前）
    CloseAudioDevice();   // T11：音频设备收尾（InitAudioDevice 成功后对称关闭）
    UnloadRenderTexture(renderTex_);
    CloseWindow();
}

void GameEngine::resetGame() {
    // T10：开局/重开统一全量重置——保证新局零残留（子弹/敌人/经验点/词条/波次账本全部清空）
    player_.reset();            // 位置/满血/Stats/武器回到初始（手枪）
    enemies_.clear();           // 清场旧敌人（防重开后残留攻击新局玩家）
    projectiles_.clear();       // 清场旧子弹（玩家弹与敌弹一并清空，防残留穿透弹）
    xpOrbs_.clear();            // 清场旧经验点
    upgrade_.reset();           // 等级 1 / 经验 0 / 升级请求清空
    waveManager_.reset();       // 波次账本复位：第 1 波、3s 休整倒计时、刷怪批次清零
    // EnemySpawner 无内部状态（出生点按玩家位置即时计算），无需复位
    ui_.resetHud();             // 血条平滑显示值复位（防旧局血量残影）
    menuCursor_ = 0;            // 菜单光标回到第一项
    survivalTime_ = 0.0f;       // 存活计时清零
    hitCount_ = 0;              // T4 调试计数清零
    killCount_ = 0;             // 击杀数清零
    noticeTimer_ = 0.0f;        // 清除残留提示文案
    noticeText_.clear();
    regenTimer_ = 0.0f;         // 生命回复 tick 计数器清零
    endStats_ = EndScreenStats{};  // 结算统计复位
    // T11：打击感与性能状态复位——清空粒子/飘字/震动，死亡定格与性能累计归零
    effects_.clear();
    grid_.clear();
    deathTimer_ = 0.0f;
    perfTimer_ = 0.0f;
    perfSteps_ = 0;
    perfMsSum_ = 0.0f;
    perfChecksSum_ = 0;
    perfCandSum_ = 0;
}

int GameEngine::handleMenuInput(int itemCount) {
    // T10：通用菜单输入——键盘方向键（上/下）移动光标，回车/空格确认；鼠标悬停高亮 + 左键确认。
    // 返回被确认项的索引（0 起）；-1 表示本帧未确认（仅移动了光标）。
    if (itemCount <= 0) return -1;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        menuCursor_ = (menuCursor_ - 1 + itemCount) % itemCount;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        menuCursor_ = (menuCursor_ + 1) % itemCount;
    }

    // 鼠标：悬停即高亮；左键点击确认
    if (hudScale_ > 0.0f) {
        const Vector2 logic = toLogicCoords(GetMousePosition());
        for (int i = 0; i < itemCount; ++i) {
            if (CheckCollisionPointRec(logic, ui_.menuRowRect(i))) {
                menuCursor_ = i;
                break;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 logic2 = toLogicCoords(GetMousePosition());
            for (int i = 0; i < itemCount; ++i) {
                if (CheckCollisionPointRec(logic2, ui_.menuRowRect(i))) return i;
            }
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) return menuCursor_;
    return -1;
}

void GameEngine::logPerf() {
    // T11：性能日志——同屏敌人 ≥ 阈值时每秒写一行 perf_log.txt（报告性能优化量化素材）。
    // 输出：存活时间/敌人数/实时 FPS/本秒平均单固定步更新耗时/每秒平均碰撞比较次数
    //（朴素版为全量次数；网格版为邻域候选数，含比较次数下界）/网格候选数。
    if (perfSteps_ <= 0) return;
    FILE* f = std::fopen("perf_log.txt", "a");
    if (f != nullptr) {
        std::fprintf(f,
                     "[PERF] t=%.1f enemies=%d fps=%d avg_step_ms=%.2f "
                     "collision_checks=%d grid_candidates=%d\n",
                     survivalTime_, static_cast<int>(enemies_.size()), GetFPS(),
                     perfMsSum_ / static_cast<float>(perfSteps_),
                     perfChecksSum_ / perfSteps_,
                     perfCandSum_ / perfSteps_);
        std::fclose(f);
    }
    perfSteps_ = 0;
    perfMsSum_ = 0.0f;
    perfChecksSum_ = 0;
    perfCandSum_ = 0;
}
