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
// T7：经验吸取→升级三选一（UPGRADE 状态世界静止）→词条应用；Stats 全联动；每秒生命回复。
// T8：波次系统（WaveManager 状态机接管刷怪）+ 喷吐者/蛮牛差异化 AI + 敌弹伤玩家。
// ============================================================
#include <memory>
#include <string>
#include <vector>

#include "raylib.h"
#include "player.h"         // GameEngine 组合持有玩家
#include "enemy.h"          // 敌人容器需要完整类型
#include "enemy_spawner.h"  // 敌人生成器（T3 F1 调试；T8 波次按类型投放）
#include "projectile.h"     // 子弹容器需要完整类型（T4）
#include "xp_orb.h"         // 经验点容器需要完整类型（T5）
#include "ui.h"             // HUD（T6）+ 升级卡片（T7）
#include "upgrade_system.h" // 经验/等级/词条三选一（T7）
#include "wave_manager.h"   // 波次状态机（T8）

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
    // T8：敌人生成器指针（WaveManager 波次投放经此委托构造，F1 调试亦复用）
    EnemySpawner* spawner = nullptr;
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
    void resolveUpgradeChoice(int idx);              // T7：应用三选一第 idx 项并决定去向
    Vector2 toLogicCoords(Vector2 winPos) const;     // T7：窗口坐标 → 逻辑坐标（鼠标判定用）

    GameState state_ = GameState::MAIN_MENU;
    float accumulator_ = 0.0f;    // 固定步长累积器（秒）
    RenderTexture2D renderTex_{}; // 1280x720 逻辑分辨率渲染目标
    Player player_{};             // 玩家（T2 接入移动）
    Camera2D camera_{};           // 跟随玩家的 2D 摄像机（仅 PLAYING 生效）
    std::vector<std::unique_ptr<Enemy>> enemies_{};  // 场上敌人容器（T3）
    EnemySpawner spawner_{};      // 敌人生成器（T3 F1 调试刷怪）
    std::vector<std::unique_ptr<Projectile>> projectiles_{};  // 场上子弹容器（T4）
    std::vector<XPOrb> xpOrbs_{};   // 场上经验点（T5：击杀掉落视觉反馈；吸取 T7）

    UIManager ui_{};         // 进行中游戏 HUD（T6）+ 升级三选一（T7）
    UpgradeSystem upgrade_{};  // T7：经验/等级/词条抽取与应用
    WaveManager waveManager_{};  // T8：波次状态机（WAITING→SPAWNING→FIGHTING）
    int hitCount_ = 0;      // T4 调试计数：子弹命中敌人次数
    int killCount_ = 0;     // T6：累计击杀数（右上角 HUD）

    std::vector<Affix> choices_{};  // T7：当前升级三选一展示
    int hoveredCard_ = -1;          // T7：鼠标悬停卡片索引（-1 无）
    std::string noticeText_{};      // T7：装备占位选择后提示文案
    float noticeTimer_ = 0.0f;      // T7：提示剩余显示时间（秒）
    float regenTimer_ = 0.0f;       // T7：每秒生命回复结算计数器

    Rectangle hudDest_{0.0f, 0.0f, 1280.0f, 720.0f}; // T7：渲染目标在窗口的映射（draw 每帧更新）
    float hudScale_ = 1.0f;         // T7：等比缩放系数（鼠标坐标逆变换用）
};
