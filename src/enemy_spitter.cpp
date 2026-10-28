// enemy_spitter.cpp —— 喷吐者实现（T8：300~600 风筝 + 2s 慢速敌弹）
#include "enemy_spitter.h"

#include <memory>  // std::make_unique

#include "assets.h"    // T12：蓝衣人贴图替代几何圆形
#include "constants.h"
#include "game.h"        // GameContext：经 ctx.projectiles 投放敌弹
#include "projectile.h"
#include "raymath.h"     // Vector2Length

Spitter::Spitter(const Vector2& spawnPos) {
    pos_ = spawnPos;
    radius_ = constants::enemy::kSpitterRadius;
    hp_ = maxHp_ = constants::enemy::kSpitterHp;
    moveSpeed_ = constants::enemy::kSpitterSpeed;
    contactDamage_ = 0.0f;   // 远程兵无近战接触伤害（威胁全部来自弹体）
    xpValue_ = constants::enemy::kSpitterXpValue;
}

void Spitter::decide(float dt, const Vector2& playerPos, GameContext& ctx) {
    const Vector2 toPlayer{playerPos.x - pos_.x, playerPos.y - pos_.y};
    const float dist = Vector2Length(toPlayer);

    // 1) 风筝走位：>600 接近 / <300 后退 / 区间内停在原地专心射击（docs/03 2.4）
    if (dist > constants::enemy::kSpitterMaxDist && dist > 0.0001f) {
        pos_.x += toPlayer.x / dist * moveSpeed_ * dt;
        pos_.y += toPlayer.y / dist * moveSpeed_ * dt;
    } else if (dist < constants::enemy::kSpitterMinDist && dist > 0.0001f) {
        pos_.x -= toPlayer.x / dist * moveSpeed_ * dt;
        pos_.y -= toPlayer.y / dist * moveSpeed_ * dt;
    }

    // 2) 射击：每 2s 朝玩家发射慢速敌弹（弹速 180 ≈ 玩家移速 0.8 倍，可走位躲开）；
    //    阵营 EnemySide——由 GameEngine 对玩家结算伤害，不参与子弹×敌人命中
    shootTimer_ -= dt;
    if (shootTimer_ <= 0.0f && ctx.projectiles != nullptr && dist > 0.0001f) {
        shootTimer_ = constants::enemy::kSpitterShootInterval;
        const Vector2 dir{toPlayer.x / dist, toPlayer.y / dist};
        ctx.projectiles->push_back(std::make_unique<Projectile>(
            pos_, dir,
            constants::enemy::kSpitterProjectileSpeed,
            constants::enemy::kSpitterProjectileDamage,
            constants::enemy::kSpitterProjectileRange,
            Projectile::Faction::EnemySide));
    }

    // 3) 无接触攻击：贴着玩家也不触发基类接触伤害（保持"炮台"定位）
    touchingPlayer_ = false;
}

void Spitter::draw() const {
    // T13：Tiny Dungeon 红色小恶魔 tile 替换 T12 蓝衣人贴图——尺寸≈2.4×碰撞半径；
    //     叠加暗紫描边环，保留"远程兵"识别色语义（与爬行者史莱姆区分）
    DrawEllipse(static_cast<int>(pos_.x), static_cast<int>(pos_.y + radius_ * 0.15f),
                radius_ * 1.0f, radius_ * 0.45f, Color{0, 0, 0, 55});
    assets::drawCentered(assets::enemySpitter(), pos_, radius_ * 2.4f, 0.0f, WHITE);
    DrawCircleLines(static_cast<int>(pos_.x), static_cast<int>(pos_.y), radius_ + 3.0f,
                    constants::enemy::kSpitterOutlineColor);
}