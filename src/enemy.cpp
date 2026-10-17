// enemy.cpp —— Enemy 基类实现（T3：非虚主调框架；T5：接触伤害/死亡标记/经验）
#include "enemy.h"

#include "game.h"  // GameContext 完整定义（接触伤害经 ctx.player 结算）

const Vector2& Enemy::pos() const { return pos_; }
float Enemy::radius() const { return radius_; }
float Enemy::hp() const { return hp_; }
bool  Enemy::touchingPlayer() const { return touchingPlayer_; }
float Enemy::contactDamage() const { return contactDamage_; }
int   Enemy::xpValue() const { return xpValue_; }
bool  Enemy::isDead() const { return isDead_; }

void Enemy::update(float dt, const Vector2& playerPos, GameContext& ctx) {
    if (isDead_) return;  // 已死亡实体本帧待移除，不再行动

    // 接触攻击冷却递减：每只敌人独立计时，避免围殴时每帧伤血（docs/03 4.3：0.8s）
    if (attackCooldown_ > 0.0f) attackCooldown_ -= dt;

    touchingPlayer_ = false;   // 每帧重置，由子类 decide 重新判定
    decide(dt, playerPos, ctx);  // 子类差异化 AI（移动/接触标记）

    // 接触伤害结算（T5）：贴脸且冷却就绪 → 玩家受伤并重置冷却；
    // 玩家侧 0.6s 无敌帧与击退在 Player::takeDamage 内统一处理
    if (touchingPlayer_ && attackCooldown_ <= 0.0f && ctx.player != nullptr) {
        ctx.player->takeDamage(contactDamage_, pos_);
        attackCooldown_ = attackInterval_;
    }
}

bool Enemy::takeDamage(float amount) {
    hp_ -= amount;
    if (hp_ <= 0.0f) {
        hp_ = 0.0f;       // 锁 0，避免负生命
        isDead_ = true;   // 死亡标志：由 GameEngine 掉落经验点并移除实体
    }
    return isDead_;
}