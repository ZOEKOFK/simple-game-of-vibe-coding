#pragma once
// ============================================================
// enemy.h —— Enemy 抽象基类
// 扩展点：新增敌种类只需继承并覆写 decide()（AI 决策）与 draw()（表现），
//         基类统一维护公共状态与"非虚主调"框架（docs/03 2.4）。
// T3 范围：状态字段、update 主调、接触标志与查询接口；
//         伤害结算/死亡标记/攻击冷却（T5）与经验掉落（T5/T8）待实现。
// ============================================================
#include "raylib.h"

struct GameContext;

class Enemy {
public:
    virtual ~Enemy() = default;

    // 非虚主调：重置接触标志 → 调用子类 AI 决策（每个定步长调用一次）
    void update(float dt, const Vector2& playerPos, GameContext& ctx);

    bool takeDamage(float amount);            // 扣血并返回是否死亡（T5 实现；本任务只保留接口）
    bool isDead() const;
    virtual void draw() const = 0;            // 子类自绘（如红色圆形，与玩家区分）

    // ---- 通用属性查询 ----
    const Vector2& pos() const;
    float radius() const;
    float hp() const;
    bool  touchingPlayer() const;             // 接触中标志（T3 标记，T5 用于伤害结算）
    float contactDamage() const;              // 接触伤害（近战/精英用，T5 启用）
    bool  attackCooldownReady(float dt);      // 接触攻击冷却框架（T5 实现）
    int   xpValue() const;                    // 死亡经验掉落（T5/T8）

protected:
    // 子类 AI 决策：每帧如何移动/如何标记接触
    //（Crawler 追踪+分离 / Spitter 放风筝 / Brute 蓄力冲锋）
    virtual void decide(float dt, const Vector2& playerPos, GameContext& ctx) = 0;

    Vector2 pos_{};                   // 世界坐标
    float   radius_ = 14.0f;          // 碰撞半径（基线见 docs/03）
    float   hp_ = 0.0f;               // 当前生命
    float   maxHp_ = 0.0f;            // 最大生命
    float   moveSpeed_ = 0.0f;        // 移速（px/s）
    float   contactDamage_ = 0.0f;    // 接触伤害
    float   attackCooldown_ = 0.0f;   // 接触攻击冷却（秒）
    bool    touchingPlayer_ = false;  // 接触中：与玩家距离 < 两半径和
};