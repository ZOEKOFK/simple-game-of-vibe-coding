#pragma once
// ============================================================
// enemy_spitter.h —— 喷吐者：远程放风筝射击怪（T8 实现）
// AI：与玩家距离 >600 接近 / <300 远离；射击冷却到点发射慢速敌弹（阵营 EnemySide）。
// 基线：HP 30、移速 100、每 2s 发射弹速 180/伤害 6 的敌弹、7 XP、无接触伤害。
// ============================================================
#include "enemy.h"

class Spitter : public Enemy {
public:
    explicit Spitter(const Vector2& spawnPos);  // 出生点由 EnemySpawner 注入

    void draw() const override;                 // 紫色圆形 + 描边（远程兵识别色）

protected:
    void decide(float dt, const Vector2& playerPos, GameContext& ctx) override;  // 风筝 + 射击

private:
    float shootTimer_ = 0.5f;  // 射击冷却剩余（秒）：出生 0.5s 后首发，避免登场即齐射
};