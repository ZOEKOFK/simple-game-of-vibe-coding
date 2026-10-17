// enemy.cpp —— Enemy 基类实现（T3：非虚主调框架与查询接口；伤害/死亡 T5）
#include "enemy.h"

#include "game.h"  // GameContext 完整定义（敌人 AI 需遍历同族列表）

const Vector2& Enemy::pos() const { return pos_; }
float Enemy::radius() const { return radius_; }
float Enemy::hp() const { return hp_; }
bool  Enemy::touchingPlayer() const { return touchingPlayer_; }
float Enemy::contactDamage() const { return contactDamage_; }

void Enemy::update(float dt, const Vector2& playerPos, GameContext& ctx) {
    touchingPlayer_ = false;   // 每帧重置，由子类 decide 重新判定
    decide(dt, playerPos, ctx);  // 子类差异化 AI（移动/接触标记）
}

bool Enemy::takeDamage(float amount) {
    // T5 实现：扣血 + 死亡标记；本任务敌人 HP 被扣时先不做任何事
    (void)amount;
    return false;
}