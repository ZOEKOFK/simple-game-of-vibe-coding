#pragma once
// ============================================================
// xp_orb.h —— 经验点实体
// 职责：掉落、停留、被玩家吸取（T5 掉落 / T7 吸取实现）。
// ============================================================
#include "raylib.h"

class XPOrb {
public:
    // T5：void draw() const;                             自绘
    // T7：void update(float dt, const Vector2& playerPos);  吸取判定

private:
    Vector2 pos_{};        // 世界坐标
    int     value_ = 5;    // 经验值（小怪 5 XP，精英倍率见 docs/03 数值表）
    bool    alive_ = true; // 存活标志
};
