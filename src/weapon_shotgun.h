#pragma once
// ============================================================
// weapon_shotgun.h —— 霰弹枪
// 开火模式：5 发 25° 扇形散射（T9 实现）。
// ============================================================
#include "weapon.h"

class Shotgun : public Weapon {
public:
    explicit Shotgun(const Stats* stats);
    ~Shotgun() override = default;

protected:
    void fire(GameContext& ctx, const Player& player, const Vector2& targetPos) override;  // T9
};
