#pragma once
// ============================================================
// enemy_crawler.h —— 爬行者：基础近战追踪怪
// AI：朝玩家方向移动；同族间距离过近施加分离偏移（避免叠成一点）。
// 基线（docs/03 4.3）：HP 20、移速 120、碰撞半径 14；接触伤害/冷却 T5 启用。
// ============================================================
#include "enemy.h"

class Crawler : public Enemy {
public:
    explicit Crawler(const Vector2& spawnPos);  // 出生点由 EnemySpawner 注入

    void draw() const override;                 // 红色圆形；接触中描边变亮黄

protected:
    void decide(float dt, const Vector2& playerPos, GameContext& ctx) override;  // 追踪 + 分离力
};