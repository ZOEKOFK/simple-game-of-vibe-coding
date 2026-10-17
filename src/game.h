#pragma once
// ============================================================
// game.h —— 引擎核心（GameEngine）
// 职责：游戏状态机 + 固定时间步主循环 + 子系统编排（后续任务逐步挂载）。
// T1 范围：窗口、固定步长主循环、五状态空壳切换。
// ============================================================
#include "raylib.h"

// ------------------------------------------------------------
// GameContext：各系统间的只读上下文总线（字段在后续任务补充）
// 完整定义放本头文件，避免新增专门头文件；其余模块一律前向声明使用。
// ------------------------------------------------------------
struct GameContext {
    // 预留：敌人容器 / 子弹容器 / 经验点容器 / 玩家引用（T3~T5 补充）
};

class GameEngine {
public:
    // ---- 游戏状态枚举（空壳）：先只声明 5 种状态 ----------------
    enum class GameState { MAIN_MENU, PLAYING, PAUSED, UPGRADE, GAME_OVER };

    void run();                       // 窗口初始化 + 主循环入口（含固定步长累积器）
    void update(float dt);            // 按状态分流更新（当前仅处理退出与状态切换）
    void draw();                      // 画面合成：渲染目标等比缩放到窗口（黑边填充）
    void changeState(GameState next); // 状态门控：唯一切换出口
    void resetGame();                 // 全量重置（T10 实现）

    GameState state() const { return state_; }

private:
    GameState state_ = GameState::MAIN_MENU;
    float accumulator_ = 0.0f;    // 固定步长累积器（秒）
    RenderTexture2D renderTex_{}; // 1280x720 逻辑分辨率渲染目标
};
