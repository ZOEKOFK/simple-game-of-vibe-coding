// ============================================================
// game.cpp —— GameEngine 实现
// T1 范围：窗口初始化、固定时间步主循环（accumulator 模式）、五状态空壳切换。
// ============================================================
#include "game.h"

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

}  // namespace

void GameEngine::changeState(GameState next) { state_ = next; }

void GameEngine::update(float dt) {
    (void)dt;  // 固定步长暂未被任何玩法逻辑使用（T2 起接入）

    // 【验收要求】Esc 键退出窗口
    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
        return;
    }

    // 数字键 1~5 切换状态：仅用于验收"五状态空壳"，后续任务替换为真实触发条件
    if (IsKeyPressed(KEY_ONE)) changeState(GameState::MAIN_MENU);
    if (IsKeyPressed(KEY_TWO)) changeState(GameState::PLAYING);
    if (IsKeyPressed(KEY_THREE)) changeState(GameState::PAUSED);
    if (IsKeyPressed(KEY_FOUR)) changeState(GameState::UPGRADE);
    if (IsKeyPressed(KEY_FIVE)) changeState(GameState::GAME_OVER);
}

void GameEngine::draw() {
    // 第 1 步：把画面绘制到 1280x720 逻辑分辨率的渲染目标
    BeginTextureMode(renderTex_);
    {
        ClearBackground(BLACK);  // 验收预期：黑色窗口
        // 空壳阶段仅显示当前状态名（后续任务由各状态的正式内容替代）
        DrawText(stateName(state_), 40, 40, 28, WHITE);
        DrawText("Esc 退出 | 数字键 1~5 切换状态", 40, 80, 20, LIGHTGRAY);
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
