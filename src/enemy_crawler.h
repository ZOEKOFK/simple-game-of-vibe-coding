#pragma once
// ============================================================
// enemy_crawler.h —— 爬行者：基础近战追踪怪（T3 实现）
// AI：朝玩家方向移动；同族间距离过近施加分离偏移。
// 基线：HP 20、移速 120、碰撞半径 14、接触伤害 8、攻击冷却 0.8s、5 XP。
// ============================================================
#include "enemy.h"

class Crawler : public Enemy {
public:
    Crawler();

    void draw() const override;  // T3

protected:
    void decide(float dt, const Vector2& playerPos, GameContext& ctx) override;  // T3
};
