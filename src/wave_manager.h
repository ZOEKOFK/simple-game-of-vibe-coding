#pragma once
// ============================================================
// wave_manager.h —— 波次管理器
// 波次状态机（休整→刷怪→清场）与数量/生命倍率曲线（T8 完整实现）。
// ============================================================

struct GameContext;

class WaveManager {
public:
    enum class State { WAITING, SPAWNING, FIGHTING };

    void update(float dt, GameContext& ctx);  // 状态推进：到刷怪时机通过 ctx 委托生成（T8）
    void startNextWave();                     // WAITING 倒计时结束进入（T8）
    void notifyEnemyDied();                   // FIGHTING 中清场判定（T8）

    int   wave() const;                       // 当前波号（UI/结算只读）
    State state() const;
    float restTimeLeft() const;
    int   totalCountForWave(int n) const;     // 数量曲线：8 + 4×(n-1)
    float hpMultiplierForWave(int n) const;   // 生命倍率：1 + 0.25×(n-1)

private:
    State state_ = State::WAITING;
    int   wave_ = 1;            // 下一波波号
    float restTimer_ = 3.0f;    // 波间休整倒计时（秒）
    int   spawnBatchLeft_ = 0;  // 本波剩余待投放数（分批双保险）
    float batchTimer_ = 0.0f;   // 批次间隔约 0.5s，避免一帧全刷
};
