// projectile.cpp —— 子弹实现（T4：飞行/射程/出界/自绘；命中伤害 T5；穿透记录 T9）
#include "projectile.h"

#include <algorithm>

#include "arena.h"
#include "constants.h"
#include "raymath.h"  // Vector2Length / Vector2Normalize

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
    // T13：弹体改程序化自绘（T12 贴图已废弃）——视觉尺寸沿 kBulletVisualSize（26px）
    //      ≈6.5×碰撞半径放大，醒目且不影响碰撞判定（判定仍只用 pos_/radius_）。
    //     子弹大而亮是用户明确要求的视觉重点。
    const bool enemy = faction_ == Faction::EnemySide;
    if (enemy) {
        // 敌弹：红紫魔法弹（外层魔光 + 深红紫外圈 + 粉内芯 + 左上高光点）
        const float R = constants::projectile::kBulletVisualSize * 0.5f;
        DrawCircleV(pos_, R, constants::projectile::kEnemyBulletGlowColor);
        DrawCircleV(pos_, R * 0.72f, constants::projectile::kEnemyBulletColor);
        DrawCircleV(pos_, R * 0.36f, constants::projectile::kEnemyBulletCoreColor);
        DrawCircleV(Vector2{pos_.x - R * 0.22f, pos_.y - R * 0.26f}, R * 0.14f, WHITE);
    } else {
        // 玩家弹：沿速度方向拉长的发光弹（尾部橙红拖尾 → 橙金弹体 → 亮白内芯）
        const Vector2 dir = Vector2Normalize(vel_);
        const Vector2 back{-dir.x, -dir.y};
        DrawCircleV(Vector2{pos_.x + back.x * 12.0f, pos_.y + back.y * 12.0f}, 3.5f,
                    constants::projectile::kBulletTrailColor);
        DrawCircleV(Vector2{pos_.x + back.x * 7.0f, pos_.y + back.y * 7.0f}, 5.0f,
                    constants::projectile::kBulletTrailColor);
        DrawCircleV(Vector2{pos_.x + dir.x * 7.0f, pos_.y + dir.y * 7.0f}, 6.0f,
                    constants::projectile::kBulletColor);
        DrawCircleV(pos_, 6.5f, constants::projectile::kBulletColor);
        DrawCircleV(Vector2{pos_.x + back.x * 4.0f, pos_.y + back.y * 4.0f}, 6.0f,
                    constants::projectile::kBulletColor);
        DrawCircleV(Vector2{pos_.x + dir.x * 6.0f, pos_.y + dir.y * 6.0f}, 3.4f,
                    constants::projectile::kBulletCoreColor);
    }
}

bool Projectile::hasHit(const Enemy* e) const {
    // T9 穿透：线性查找已命中记录（子弹寿命短、记录极少，无性能压力）
    return std::find(hits_.begin(), hits_.end(), e) != hits_.end();
}

void Projectile::recordHit(const Enemy* e) {
    hits_.push_back(e);  // T9 穿透：防止同一子弹多帧内对同一敌人重复结算
}