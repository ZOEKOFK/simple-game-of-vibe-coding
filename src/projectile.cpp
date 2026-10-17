// projectile.cpp —— 子弹实现（T4：飞行/射程/出界/自绘；命中伤害 T5）
#include "projectile.h"

#include "arena.h"
#include "constants.h"
#include "raymath.h"  // Vector2Length

Projectile::Projectile(const Vector2& pos, const Vector2& dir, float speed,
                       float damage, float maxRange) {
    pos_ = pos;
    vel_ = Vector2{dir.x * speed, dir.y * speed};
    radius_ = constants::projectile::kBulletRadius;
    damage_ = damage;
    rangeLeft_ = maxRange;
}

void Projectile::update(float dt) {
    if (!alive_) return;
    pos_.x += vel_.x * dt;
    pos_.y += vel_.y * dt;

    // 射程衰减：飞满 maxRange 自动消失，子弹不会无限飞行
    rangeLeft_ -= Vector2Length(vel_) * dt;
    if (rangeLeft_ <= 0.0f) {
        alive_ = false;
        return;
    }
    // 出界失效：飞出竞技场矩形即移除（docs/02 3.3 生命周期自检）
    if (pos_.x < 0.0f || pos_.x > constants::arena::kWidth ||
        pos_.y < 0.0f || pos_.y > constants::arena::kHeight) {
        alive_ = false;
    }
}

void Projectile::draw() const {
    if (!alive_) return;
    DrawCircleV(pos_, radius_, constants::projectile::kBulletColor);
}