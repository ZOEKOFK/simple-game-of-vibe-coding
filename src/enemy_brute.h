#pragma once
// ============================================================
// enemy_brute.h —— 蛮牛：精英冲锋怪（T8 实现）
// AI 状态机：追踪 →（进入 320px）蓄力 0.8s（钉住原地、变红闪烁、方向箭头预警）
//           → 冲锋 1s（速度×3，沿蓄力期锁定方向直线冲刺）→ 疲惫冷却 5s（恢复追踪），循环。
// 基线：HP 600(×30)、移速 84(×0.7)、体型 28(×2)、接触伤害 24(×3)、经验 50(×10)。（第 8 波起）
// ============================================================
#include "enemy.h"

class Brute : public Enemy {
public:
    explicit Brute(const Vector2& spawnPos);  // 出生点由 EnemySpawner 注入

    void draw() const override;               // 深红大圆；蓄力变红闪烁+方向箭头；冲锋亮红+尾迹
    bool isElite() const override { return true; }  // T11：精英——死亡时触发更强屏幕震动

protected:
    void decide(float dt, const Vector2& playerPos, GameContext& ctx) override;  // 蓄力冲锋状态机

private:
    enum class Phase { Chase, Windup, Charge, Cooldown };  // 追踪/蓄力/冲锋/疲惫

    Phase  phase_ = Phase::Chase;   // 当前状态
    Vector2 chargeDir_{1.0f, 0.0f}; // 冲锋方向：蓄力期持续锁定玩家方位，冲锋期冻结直线冲刺
    float windupTimer_   = 0.0f;    // 蓄力剩余（秒）
    float chargeTimer_   = 0.0f;    // 冲锋剩余（秒）
    float cooldownTimer_ = 0.0f;    // 冲锋冷却剩余（秒）
};