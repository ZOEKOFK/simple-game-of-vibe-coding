#pragma once
// ============================================================
// enemy_spawner.h —— 敌人生成器
// 职责：出生位置规则与敌人构造；返回敌人列表交给 GameEngine 实体容器。
// T3 临时版：spawnWave(count) 刷爬行者（F1 调试）；正式波次构成与倍率 T8 接入。
// ============================================================
#include <memory>
#include <vector>

#include "enemy.h"   // unique_ptr<Enemy> 析构需要完整类型
#include "raylib.h"  // Vector2

class EnemySpawner {
public:
    // 一次生成 count 只爬行者：竞技场边缘内侧随机出生，距玩家 ≥ kSpawnMinPlayerDist
    std::vector<std::unique_ptr<Enemy>> spawnWave(int count, const Vector2& playerPos);

private:
    Vector2 pickSpawnPos(const Vector2& playerPos) const;  // 出生点计算（边缘 + 安全距离）
};