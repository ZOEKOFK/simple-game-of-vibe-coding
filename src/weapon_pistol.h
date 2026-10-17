#pragma once
// ============================================================
// weapon_pistol.h —— 手枪（初始武器）
// 开火模式：向索敌目标方向发射单发子弹（T4 实现）。
// 基线：射速 2.4 发/秒、伤害 10、弹速 600、射程 900（见 docs/03 数值表）。
// ============================================================
#include "weapon.h"

class Pistol : public Weapon {
public:
    explicit Pistol(const Stats* stats);
    ~Pistol() override = default;

protected:
    void fire(GameContext& ctx, const Player& player, const Vector2& targetPos) override;  // T4
};
