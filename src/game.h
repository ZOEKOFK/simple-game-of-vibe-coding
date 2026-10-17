#pragma once
// ============================================================
// game.h —— 引擎核心（GameEngine）
// 职责：游戏状态机 + 固定时间步主循环 + 子系统编排（后续任务逐步挂载）。
// T1：窗口、固定步长主循环、五状态空壳切换。
// T2：挂载 Player 与跟随摄像机（仅 PLAYING 生效，其余状态维持 T1 表现）。
// T3：F1 调试刷怪 + 敌人 AI 更新与绘制。
// T4：武器自动索敌/开火、子弹飞行与命中判定（命中暂只移除子弹）。
// T5：伤害结算（子弹-敌人/敌人-玩家）、击杀移除、经验点掉落与玩家受击反馈。
// T6：HUD（等级/经验条/血条/波次占位/敌人/击杀/武器说明，屏幕空间绘制）。
// ============================================================
#include <memory>
#include <vector>

#include "raylib.h"
#include "player.h"         // GameEngine 组合持有玩家
#include "enemy.h"          // 敌人容器需要完整类型
#include "enemy_spawner.h"  // T3 调试刷怪
#include "projectile.h"     // 子弹容器需要完整类型（T4）
#include "xp_orb.h"         // 经验点容器需要完整类型（T5）
#include "ui.h"             // HUD（T6）

// ------------------------------------------------------------
// GameContext：各系统间的只读上下文总线（字段在后续任务补充）
// 完整定义放本头文件，避免新增专门头文件；其余模块一律前向声明使用。
// ------------------------------------------------------------
struct GameContext {
    // T3：敌人列表指针（供敌人 AI 分离力等只读遍历；由 GameEngine 更新时填充）
    std::vector<std::unique_ptr<Enemy>>* enemies = nullptr;
    // T4：子弹容器指针（武器开火生成子弹的唯一落点，由 GameEngine 统一管理）
    std::vector<std::unique_ptr<Projectile>>* projectiles = nullptr;
    // T5：玩家指针（敌人接触攻击经此结算伤害）与经验点容器（击杀掉落落点，吸取 T7）
    Player* player = nullptr;
    std::vector<XPOrb>* xpOrbs = nullptr;
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
    Player player_{};             // 玩家（T2 接入移动）
    Camera2D camera_{};           // 跟随玩家的 2D 摄像机（仅 PLAYING 生效）
    std::vector<std::unique_ptr<Enemy>> enemies_{};  // 场上敌人容器（T3）
    EnemySpawner spawner_{};      // 敌人生成器（T3 F1 调试刷怪）
    std::vector<std::unique_ptr<Projectile>> projectiles_{};  // 场上子弹容器（T4）
    std::vector<XPOrb> xpOrbs_{};   // 场上经验点（T5：击杀掉落视觉反馈；吸取 T7）

    UIManager ui_{};         // 进行中游戏 HUD（T6：经验/血条/波次/击杀/武器说明）
    int hitCount_ = 0;      // T4 调试计数：子弹命中敌人次数
    int killCount_ = 0;     // T6：累计击杀数（右上角 HUD）
    int wave_ = 1;          // T6 占位：当前波次（T8 波次系统接管）
    float waveTimer_ = 0.0f; // T6 占位：波次计时 0（T8 接管）
    int level_ = 1;         // T6 显示：玩家等级（经验/升级逻辑 T7 迁 UpgradeSystem）
    int exp_ = 0;           // T6 显示：当前等级内经验（拾取 T7 累积）
};
