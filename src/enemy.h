#pragma once
// ============================================================
// enemy.h —— Enemy 抽象基类
// 扩展点：新增敌种类只需继承并覆写 decide()（AI 决策）与 draw()（表现），
//         基类统一维护公共状态与"非虚主调"框架（docs/03 2.4）。
// T3：状态字段、update 主调、接触标志与查询接口；
// T5：接触攻击冷却与伤害结算、takeDamage 扣血/死亡标记、经验掉落。
// ============================================================
#include "constants.h"  // 数值基线（含 T11 fx 常量）
#include "raylib.h"

struct GameContext;

class Enemy {
public:
    virtual ~Enemy() = default;

    // 非虚主调：重置接触标志 → 维护攻击冷却 → 调用子类 AI 决策 → 接触伤害结算
    void update(float dt, const Vector2& playerPos, GameContext& ctx);

    bool takeDamage(float amount);            // T5：扣血并返回是否死亡（hp≤0 锁定 0 并置死亡标志）
    bool isDead() const;
    virtual void draw() const = 0;            // 子类自绘（如红色圆形，与玩家区分）
    void scaleHp(float multiplier);           // T8：波次生命倍率（出生时调用一次：maxHp/hp 同乘）

    // T11：受击闪白——takeDamage 触发闪白计时，返回 0~1 剩余强度供绘制叠加白色半透明层
    float hitFlashAlpha() const {
        return (hitFlashTimer_ > 0.0f)
                   ? hitFlashTimer_ / constants::fx::kHitFlashDuration
                   : 0.0f;
    }
    // T11：精英标记——蛮牛等精英死亡时触发更强震动（基类默认非精英）
    virtual bool isElite() const { return false; }

    // ---- 通用属性查询 ----
    const Vector2& pos() const;
    float radius() const;
    float hp() const;
    bool  touchingPlayer() const;             // 接触中标志（T3 标记，T5 用于接触伤害结算）
    float contactDamage() const;              // 接触伤害（近战/精英用，T5 启用）
    int   xpValue() const;                    // 死亡经验掉落（T5 掉落展示，吸取 T7）

protected:
    // 子类 AI 决策：每帧如何移动/如何标记接触
    //（Crawler 追踪+分离 / Spitter 放风筝 / Brute 蓄力冲锋）
    virtual void decide(float dt, const Vector2& playerPos, GameContext& ctx) = 0;

    Vector2 pos_{};                   // 世界坐标
    float   radius_ = 14.0f;          // 碰撞半径（基线见 docs/03）
    float   hp_ = 0.0f;               // 当前生命
    float   maxHp_ = 0.0f;            // 最大生命
    float   moveSpeed_ = 0.0f;        // 移速（px/s）
    float   contactDamage_ = 0.0f;    // 接触伤害（子类构造时按数值表设置）
    float   attackCooldown_ = 0.0f;   // 接触攻击冷却剩余（秒，>0 表示冷却中）
    float   attackInterval_ = 0.8f;   // 接触攻击间隔（秒，子类构造时按数值表设置）
    int     xpValue_ = 0;             // 击杀经验掉落
    bool    isDead_ = false;          // 死亡标志（hp≤0 置位，由 GameEngine 移除实体）
    bool    touchingPlayer_ = false;  // 接触中：与玩家距离 < 两半径和
    float   hitFlashTimer_ = 0.0f;    // T11：受击闪白剩余时长（秒，>0 时绘制白色叠加层）
};