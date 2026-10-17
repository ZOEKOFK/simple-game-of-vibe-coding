#pragma once
// ============================================================
// enemy.h —— Enemy 抽象基类
// 通用状态与受击框架在基类；AI 决策通过虚函数交由子类（T3 实现）。
// ============================================================
#include "raylib.h"

struct GameContext;

class Enemy {
public:
    virtual ~Enemy() = default;

    bool takeDamage(float amount);       // 扣血并返回是否死亡（T5）
    bool isDead() const;
    void update(float dt, const Vector2& playerPos, GameContext& ctx);  // 非虚主调（T3）
    virtual void draw() const = 0;

    // ---- 通用属性查询 ----
    const Vector2& pos() const;
    float radius() const;
    float hp() const;
    float contactDamage() const;       // 接触伤害（近战/精英用）
    bool  attackCooldownReady(float dt);  // 接触攻击冷却框架（T3）
    int   xpValue() const;             // 死亡经验掉落

protected:
    // AI 决策：Crawler 追踪 / Spitter 放风筝 / Brute 冲锋（T3/T8）
    virtual void decide(float dt, const Vector2& playerPos, GameContext& ctx) = 0;

    Vector2 pos_{};                   // 世界坐标
    float   radius_ = 14.0f;          // 碰撞半径（基线见 docs/03）
    float   hp_ = 0.0f;               // 当前生命
    float   maxHp_ = 0.0f;            // 最大生命
    float   moveSpeed_ = 0.0f;        // 移速
    float   contactDamage_ = 0.0f;    // 接触伤害
    float   attackCooldown_ = 0.0f;   // 接触攻击冷却（秒）
};
