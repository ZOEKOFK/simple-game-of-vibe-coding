#pragma once
// ============================================================
// wave_manager.h —— 波次管理器（T8 完整实现）
// 状态机：WAITING（3s 休整倒计时）→ SPAWNING（0.5s 分批投放）→ FIGHTING（清场判定回 WAITING）。
// 数量/生命倍率曲线与敌人构成权重见 docs/03 4.4，全部常量收编 constants::wave。
// ============================================================

#include "constants.h"       // constants::wave（波次数值基线）
#include "enemy_spawner.h"   // EnemyType：构成随机分发

struct GameContext;

class WaveManager {
public:
    enum class State { WAITING, SPAWNING, FIGHTING };

    void update(float dt, GameContext& ctx);  // 状态推进；SPAWNING 经 ctx.spawner 委托生成
    void reset();                             // T10：新局复位（第 1 波/3s 休整/账本清零）
    void startNextWave();                     // WAITING 倒计时结束 → SPAWNING（不供外部直接调用）
    void notifyEnemyDied();                   // 本波存活账本递减（清场判定；F1 调试怪同样记账）

    int   wave() const;                       // 当前波号（WAITING 期间即"下一波"波号，UI 只读）
    State state() const;
    float restTimeLeft() const;               // WAITING 剩余倒计时（秒，HUD 显示）
    int   remainingToSpawn() const;           // SPAWNING 本波剩余待投放数（HUD 显示）
    int   aliveLeft() const;                  // 本波存活账本（FIGHTING 清场判定/调试只读）
    int   totalCountForWave(int n) const;     // 数量曲线：8 + 4×(n-1)
    float hpMultiplierForWave(int n) const;   // 生命倍率：1 + 0.25×(n-1)

private:
    EnemyType rollType();                       // 按当前波次构成权重随机类型（蛮牛每波 ≤ 2 只）

    State state_ = State::WAITING;
    int   wave_  = 1;                        // 当前波号（开局 WAITING 即为第 1 波倒计时）
    float restTimer_ = constants::wave::kRestTime;  // 波间休整倒计时（秒）
    int   spawnBatchLeft_ = 0;               // 本波剩余待投放数（分批双保险）
    float batchTimer_ = 0.0f;                // 批次间隔 0.5s，避免一帧全刷
    int   aliveLeft_ = 0;                    // 本波存活账本：spawn ++ / 死亡 --（清场判定）
    int   brutesSpawned_ = 0;                // 本波已投放蛮牛数（每波上限约束）
};