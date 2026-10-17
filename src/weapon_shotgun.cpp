// weapon_shotgun.cpp —— 霰弹枪实现与工厂自注册（T9）
#include "weapon_shotgun.h"

#include <memory>

#include "constants.h"
#include "game.h"        // GameContext：子弹容器
#include "player.h"      // Player 位置查询
#include "projectile.h"
#include "raymath.h"     // Vector2Length / Vector2Rotate
#include "stats.h"       // 伤害/弹速倍率（T7 词条联动）

namespace {

// T9 工厂自注册：登记 "shotgun" → 构造器，装备词条经 Weapon::create 实装
const bool kShotgunRegistered = []() {
    Weapon::registerWeapon("shotgun", [](const Stats* stats) {
        return std::make_unique<Shotgun>(stats);
    });
    return true;
}();

}  // namespace

Shotgun::Shotgun(const Stats* stats)
    : Weapon(constants::weapon::kShotgunFireInterval,
             constants::weapon::kShotgunDamage,
             constants::weapon::kShotgunRange) {
    stats_ = stats;
    typeId_ = "shotgun";
}

std::string Shotgun::displayName() const { return "霰弹枪"; }
std::string Shotgun::description() const { return "5 发扇形散射"; }

float Shotgun::upgradeDamageAmount() const {
    // 5 弹丸齐射，统一 +6 会造成 +50% 齐射增伤（vs 手枪 +25%），压至每弹丸 +3（docs/05 §4.1）
    return constants::weapon::kShotgunUpgradeDamage;
}

void Shotgun::fire(GameContext& ctx, const Player& player, const Vector2& targetPos) {
    if (ctx.projectiles == nullptr) return;

    // 中轴方向：玩家 → 索敌目标（归一化，含重合兜底）
    Vector2 baseDir{targetPos.x - player.pos().x, targetPos.y - player.pos().y};
    const float len = Vector2Length(baseDir);
    if (len > 0.0001f) {
        baseDir.x /= len;
        baseDir.y /= len;
    } else {
        baseDir = Vector2{1.0f, 0.0f};
    }

    // 伤害/弹速联动 Stats（伤害强化、高速子弹生效；基础伤害随"装备词条再选"升级成长）
    const float damage = baseDamage_ * (stats_ != nullptr ? stats_->damageMult : 1.0f);
    const float bulletSpeed = constants::projectile::kBulletSpeed *
                              (stats_ != nullptr ? stats_->bulletSpeedMult : 1.0f);

    // 5 发扇形：总开角 25°，弹丸间夹角 25/4 = 6.25°，依次偏移 -12.5° ~ +12.5°
    const float angleStep = constants::weapon::kShotgunSpreadDeg * DEG2RAD /
                            static_cast<float>(constants::weapon::kShotgunPelletCount - 1);
    const float halfAngle = constants::weapon::kShotgunSpreadDeg * 0.5f * DEG2RAD;
    for (int i = 0; i < constants::weapon::kShotgunPelletCount; ++i) {
        const Vector2 dir = Vector2Rotate(baseDir, -halfAngle + angleStep * i);
        const Vector2 muzzle{player.pos().x + dir.x * constants::weapon::kMuzzleOffset,
                             player.pos().y + dir.y * constants::weapon::kMuzzleOffset};
        // 弹丸射程 = 索敌射程 500（近距离武器定位：远了弹丸自然消散）
        auto bullet = std::make_unique<Projectile>(muzzle, dir, bulletSpeed, damage,
                                                   constants::weapon::kShotgunRange);
        // T9 穿透：穿甲弹对每颗弹丸独立生效（平衡风险见 docs/05）
        bullet->setPierce(stats_ != nullptr ? stats_->penetrationExtra : 0);
        ctx.projectiles->push_back(std::move(bullet));
    }
}