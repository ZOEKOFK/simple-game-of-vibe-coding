#pragma once
// ============================================================
// projectile.h —— 子弹与敌方弹体（含阵营标志）
// T4：玩家子弹的飞行/射程衰减/出界失效/自绘；命中造成的伤害 T5 结算；
//     敌方弹体（EnemySide）在 T8 喷吐者接入。
// ============================================================
#include "raylib.h"

class Projectile {
public:
    enum class Faction { PlayerSide, EnemySide };  // 阵营标志：区分敌我子弹

    // pos 起点；dir 须为单位方向；speed 弹速；maxRange 最大射程（超出自动失效）
    Projectile(const Vector2& pos, const Vector2& dir, float speed,
               float damage, float maxRange);

    void update(float dt);   // 飞行 + 射程衰减 + 出界失效
    void draw() const;       // 黄色小圆（T4 玩家弹表现）

    // ---- 查询接口（碰撞检测/伤害结算用）----
    const Vector2& pos() const { return pos_; }
    float radius() const { return radius_; }
    float damage() const { return damage_; }
    Faction faction() const { return faction_; }
    bool  alive() const { return alive_; }
    void  markDead() { alive_ = false; }   // 命中/失效后由 GameEngine 标记移除

private:
    Vector2 pos_{};                        // 世界坐标
    Vector2 vel_{};                        // 速度（px/s，含方向）
    float   radius_ = 4.0f;                // 碰撞半径
    float   damage_ = 0.0f;                // 伤害（命中结算用，T5）
    float   rangeLeft_ = 0.0f;             // 剩余射程（超出衰减消失）
    Faction faction_ = Faction::PlayerSide;
    bool    alive_ = true;                 // 存活标志（命中/超射程/出界后置 false）
};