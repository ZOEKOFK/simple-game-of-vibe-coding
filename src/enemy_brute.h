#pragma once
// ============================================================
// enemy_brute.h —— 蛮牛：精英冲锋怪（T8 实现）
// AI 状态机：追踪 → 蓄力 0.8s（变红）→ 冲锋 1s（速度×3）→ 冷却 5s，循环。
// 基线：HP×30、移速×0.7、体型×2、接触伤害×3、死亡经验×10。（第 8 波起）
// ============================================================
#include "enemy.h"

class Brute : public Enemy {
public:
    Brute();

    void draw() const override;  // T8

protected:
    void decide(float dt, const Vector2& playerPos, GameContext& ctx) override;  // T8
};
