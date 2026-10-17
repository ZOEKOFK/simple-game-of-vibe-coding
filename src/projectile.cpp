// projectile.cpp —— 子弹实现（T4：飞行/射程/出界/自绘；命中伤害 T5；穿透记录 T9）
#include "projectile.h"

#include <algorithm>

#include "arena.h"
#include "assets.h"    // T12：弹丸贴图替代几何圆形
#include "constants.h"
#include "raymath.h"  // Vector2Length

Projectile::Projectile(const Vector2& pos, const Vector2& dir, float speed,
                       float damage, float maxRange, Faction faction) {
    pos_ = pos;
    vel_ = Vector2{dir.x * speed, dir.y * speed};
    radius_ = constants::projectile::kBulletRadius;
    damage_ = damage;
    rangeLeft_ = maxRange;
    faction_ = faction;
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
    // T12：弹丸贴图替代几何圆形——阵营区分靠 tint（玩家黄 flare / 敌方红 circle），
    //     贴图等比放大至≈2.2×碰撞半径，视觉略大于碰撞盒
    const bool enemy = faction_ == Faction::EnemySide;
    const Texture2D tex = enemy ? assets::enemyBullet() : assets::bullet();
    assets::drawCentered(tex, pos_, radius_ * 2.2f, 0.0f,
                         enemy ? constants::projectile::kEnemyBulletColor
                               : constants::projectile::kBulletColor);
}

bool Projectile::hasHit(const Enemy* e) const {
    // T9 穿透：线性查找已命中记录（子弹寿命短、记录极少，无性能压力）
    return std::find(hits_.begin(), hits_.end(), e) != hits_.end();
}

void Projectile::recordHit(const Enemy* e) {
    hits_.push_back(e);  // T9 穿透：防止同一子弹多帧内对同一敌人重复结算
}