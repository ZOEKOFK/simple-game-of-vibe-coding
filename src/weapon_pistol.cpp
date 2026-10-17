// weapon_pistol.cpp —— 手枪实现（T4：单发开火；T9：工厂自注册 + 穿透接入）
#include "weapon_pistol.h"

#include <memory>

#include "constants.h"
#include "game.h"        // GameContext：子弹容器
#include "player.h"      // Player 位置/属性
#include "projectile.h"
#include "raymath.h"     // Vector2Length
#include "stats.h"       // 伤害倍率（默认 1.0）

namespace {

// T9 工厂自注册：武器子类在静态初始化期登记"注册名 → 构造器"，新增武器零改动旧文件
const bool kPistolRegistered = []() {
    Weapon::registerWeapon("pistol", [](const Stats* stats) {
        return std::make_unique<Pistol>(stats);
    });
    return true;
}();

}  // namespace

Pistol::Pistol(const Stats* stats)
    : Weapon(constants::weapon::kPistolFireInterval,
             constants::weapon::kPistolDamage,
             constants::weapon::kPistolRange) {
    stats_ = stats;       // 只读引用：伤害 = 基础伤害 × damageMult（词条 T7 生效）
    typeId_ = "pistol";  // 注册名（UI 展示/强化判重用）
}

void Pistol::fire(GameContext& ctx, const Player& player, const Vector2& targetPos) {
    if (ctx.projectiles == nullptr) return;

    // 枪口朝向锁定索敌目标：从玩家圆心指向目标归一化（无误差偏移）
    Vector2 dir{targetPos.x - player.pos().x, targetPos.y - player.pos().y};
    const float len = Vector2Length(dir);
    if (len > 0.0001f) {
        dir.x /= len;
        dir.y /= len;
    } else {
        dir = Vector2{1.0f, 0.0f};  // 与目标重合时兜底朝右
    }

    const Vector2 muzzle{player.pos().x + dir.x * constants::weapon::kMuzzleOffset,
                         player.pos().y + dir.y * constants::weapon::kMuzzleOffset};

    // 伤害 = 基础伤害 × Stats 伤害倍率（T9：基础伤害随"装备词条再选"升级 +6 成长）
    const float damage = baseDamage_ * (stats_ != nullptr ? stats_->damageMult : 1.0f);

    // 弹速 = 基础弹速 × Stats 弹速倍率（T7 词条联动："高速子弹"生效）
    const float bulletSpeed = constants::projectile::kBulletSpeed *
                              (stats_ != nullptr ? stats_->bulletSpeedMult : 1.0f);

    // 单发：向目标方向创建一颗子弹，交给 GameEngine 统一管理；
    // T9 穿透：穿甲弹词条的可穿透数写入子弹（穿透 0 时命中即消失，与 T8 行为一致）
    auto bullet = std::make_unique<Projectile>(
        muzzle, dir, bulletSpeed, damage, constants::projectile::kBulletMaxRange);
    bullet->setPierce(stats_ != nullptr ? stats_->penetrationExtra : 0);
    ctx.projectiles->push_back(std::move(bullet));
}