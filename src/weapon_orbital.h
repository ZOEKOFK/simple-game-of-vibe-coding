#pragma once
// ============================================================
// weapon_orbital.h —— 环绕飞刃（T9）
// 无"开火"语义：刃体绕玩家匀速旋转（1.8 rad/s、半径 90），
//   对接触敌人每 0.5 秒至多结算一次伤害 15（每敌独立冷却字典）。
// 基线（docs/03 4.3）：单刃伤害 15、冷却 0.5s、初始 2 刃。
// ============================================================
#include <unordered_map>

#include "constants.h"  // 刃数/冷却等数值基线
#include "weapon.h"

class Enemy;  // 前向声明：仅作冷却字典键

class OrbitalBlade : public Weapon {
public:
    explicit OrbitalBlade(const Stats* stats);
    ~OrbitalBlade() override = default;

    // 整体覆写主流程：旋转 + 每刃接触判定 + 每敌 0.5s 冷却（不参与索敌/攻速）
    void update(float dt, GameContext& ctx, const Player& player) override;
    void draw(const Player& player) const override;  // 刃体自绘（世界坐标，含光晕）

    // ---- 展示信息（T6 HUD 底部武器说明；刃数动态描述）----
    std::string displayName() const override { return "环绕飞刃"; }
    std::string description() const override;

    // 复选升级：2 刃 0.5s 高频结算，增量压至单刃 +4，避免站桩收益超模（docs/05 §4.1）
    float upgradeDamageAmount() const override;

private:
    float angle_ = 0.0f;  // 当前旋转相位（rad），均匀间隔分布多刃
    int   bladeCount_ = constants::weapon::kOrbitalBladeCount;  // 刃数（基线 2）
    // 每敌伤害冷却字典：敌人指针 → 剩余冷却（秒）；每敌每 0.5s 至多结算一次
    std::unordered_map<const Enemy*, float> hitCooldowns_{};
};