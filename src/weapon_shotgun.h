#pragma once
// ============================================================
// weapon_shotgun.h —— 霰弹枪（T9）
// 开火模式：5 发 25° 扇形散射（±12.5° 均匀分布）；贴脸全中伤害 60。
// 基线（docs/03 4.3）：单发伤害 12、射速 0.8 发/秒、射程 500。
// ============================================================
#include "weapon.h"

class Shotgun : public Weapon {
public:
    explicit Shotgun(const Stats* stats);
    ~Shotgun() override = default;

    // ---- 展示信息（T6 HUD 底部武器说明）----
    std::string displayName() const override;  // "霰弹枪"
    std::string description() const override;  // "5 发扇形散射"

    // 复选升级：5 弹丸齐射，增量压至每弹丸 +3，保证与手枪复选收益对齐（docs/05 §4.1）
    float upgradeDamageAmount() const override;

protected:
    void fire(GameContext& ctx, const Player& player, const Vector2& targetPos) override;
};