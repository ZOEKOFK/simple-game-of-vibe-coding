#pragma once
// ============================================================
// projectile.h —— 子弹与敌方弹体（含阵营标志）
// T1 仅声明；飞行/命中/穿透逻辑在 T4/T5 实现。
// ============================================================
#include "raylib.h"

class Projectile {
public:
    enum class Faction { PlayerSide, EnemySide };  // 阵营标志：区分敌我子弹

    // 后续任务补全：飞行更新、命中判定、穿透计数、射程衰减……

private:
    Vector2 pos_{};                     // 世界坐标
    Vector2 vel_{};                     // 速度（像素/秒）
    float   damage_ = 0.0f;             // 伤害
    float   rangeLeft_ = 0.0f;          // 剩余射程（超出衰减或消失）
    Faction faction_ = Faction::PlayerSide;
    bool    alive_ = true;              // 存活标志（命中/超射程后置 false）
};
