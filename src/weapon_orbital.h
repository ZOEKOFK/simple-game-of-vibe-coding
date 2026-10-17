#pragma once
// ============================================================
// weapon_orbital.h —— 环绕飞刃
// 无"开火"语义：持续绕体旋转的刃体 + 周期接触伤害，整体覆写 update()（T9 实现）。
// ============================================================
#include "weapon.h"

class OrbitalBlade : public Weapon {
public:
    explicit OrbitalBlade(const Stats* stats);
    ~OrbitalBlade() override = default;

    void update(float dt, GameContext& ctx, const Player& player) override;  // T9
};
