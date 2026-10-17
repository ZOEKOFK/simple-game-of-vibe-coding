// weapon_orbital.cpp —— 环绕飞刃实现与工厂自注册（T9）
#include "weapon_orbital.h"

#include <cmath>
#include <memory>
#include <string>

#include "collision.h"  // 圆-圆接触判定（T5 统一接口）
#include "enemy.h"      // Enemy：位置/半径/takeDamage/isDead
#include "game.h"       // GameContext：敌人遍历 + 玩家指针（吸血结算）
#include "player.h"
#include "raymath.h"    // PI（旋转角度计算）

namespace {

// T9 工厂自注册：登记 "orbital" → 构造器，装备词条经 Weapon::create 实装
const bool kOrbitalRegistered = []() {
    Weapon::registerWeapon("orbital", [](const Stats* stats) {
        return std::make_unique<OrbitalBlade>(stats);
    });
    return true;
}();

}  // namespace

OrbitalBlade::OrbitalBlade(const Stats* stats)
    : Weapon(0.0f /* 无开火间隔：整体覆写 update */,
             constants::weapon::kOrbitalBladeDamage, 0.0f) {
    stats_ = stats;
    typeId_ = "orbital";
}

std::string OrbitalBlade::description() const {
    return std::to_string(bladeCount_) + " 刃环绕";  // 刃数动态展示（如"2 刃环绕"）
}

float OrbitalBlade::upgradeDamageAmount() const {
    // 2 刃 ×0.5s 高频结算，统一 +6 会造成 +40% 单刃增伤且与吸血相乘，压至 +4（docs/05 §4.1）
    return constants::weapon::kOrbitalUpgradeDamage;
}

void OrbitalBlade::update(float dt, GameContext& ctx, const Player& player) {
    if (ctx.enemies == nullptr) return;

    // 1) 匀速旋转：相位推进后按刃数均匀分布各刃角
    angle_ += constants::weapon::kOrbitalAngularSpeed * dt;

    // 2) 冷却字典衰减：全部 -dt，到期条目即时移除（防敌人死亡后条目累积）
    for (auto it = hitCooldowns_.begin(); it != hitCooldowns_.end();) {
        it->second -= dt;
        if (it->second <= 0.0f) {
            it = hitCooldowns_.erase(it);
        } else {
            ++it;
        }
    }

    // 3) 伤害联动：基础 + 飞刃强化（加算）× 伤害强化（乘算）；
    //    注意攻速词条不作用于飞刃（无开火间隔），该取舍见 docs/05 平衡说明
    const float rawDamage = baseDamage_ +
                            (stats_ != nullptr ? stats_->bladeDamageBonus : 0.0f);
    const float damage = rawDamage *
                         (stats_ != nullptr ? stats_->damageMult : 1.0f);
    const float lifesteal = stats_ != nullptr ? stats_->lifestealPct : 0.0f;

    const float bladeStep = 2.0f * PI / static_cast<float>(bladeCount_);
    for (int i = 0; i < bladeCount_; ++i) {
        const float a = angle_ + bladeStep * i;
        const Vector2 bladePos{player.pos().x + std::cos(a) * constants::weapon::kOrbitalRadius,
                               player.pos().y + std::sin(a) * constants::weapon::kOrbitalRadius};

        // 4) 接触判定：每把刃独立扫全部敌人；同一敌人靠冷却字典保证"每半秒至多一次"
        for (const auto& e : *ctx.enemies) {
            if (!e || e->isDead()) continue;
            if (hitCooldowns_.count(e.get()) > 0) continue;  // 该敌尚在 0.5s 冷却内
            if (!collision::circlesOverlap(bladePos, constants::weapon::kOrbitalBladeRadius,
                                           e->pos(), e->radius())) {
                continue;
            }
            e->takeDamage(damage);  // 击杀的账本/掉落由 GameEngine 帧末统一结算（T9）
            hitCooldowns_[e.get()] = constants::weapon::kOrbitalHitInterval;
            if (lifesteal > 0.0f && ctx.player != nullptr) {
                ctx.player->heal(damage * lifesteal);  // T9 吸血：按结算伤害比例回复
            }
        }
    }
}

void OrbitalBlade::draw(const Player& player) const {
    const float bladeStep = 2.0f * PI / static_cast<float>(bladeCount_);
    for (int i = 0; i < bladeCount_; ++i) {
        const float a = angle_ + bladeStep * i;
        const Vector2 bladePos{player.pos().x + std::cos(a) * constants::weapon::kOrbitalRadius,
                               player.pos().y + std::sin(a) * constants::weapon::kOrbitalRadius};
        // 光晕提示杀伤范围 + 银白刃体
        DrawCircleV(bladePos, constants::weapon::kOrbitalBladeRadius * 2.2f,
                    constants::weapon::kOrbitalBladeGlow);
        DrawCircleV(bladePos, constants::weapon::kOrbitalBladeRadius,
                    constants::weapon::kOrbitalBladeColor);
        DrawCircleLines(static_cast<int>(bladePos.x), static_cast<int>(bladePos.y),
                        static_cast<int>(constants::weapon::kOrbitalBladeRadius),
                        DARKGRAY);
    }
}