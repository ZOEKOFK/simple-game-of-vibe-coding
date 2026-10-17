#pragma once
// ============================================================
// enemy_spitter.h —— 喷吐者：远程放风筝射击怪（T8 实现）
// AI：与玩家距离 >600 接近 / <300 远离；射击冷却到点发射慢速敌弹。
// 基线：HP 30、移速 100、每 2s 发射弹速 180/伤害 6 的敌弹、7 XP。
// ============================================================
#include "enemy.h"

class Spitter : public Enemy {
public:
    Spitter();

    void draw() const override;  // T8

protected:
    void decide(float dt, const Vector2& playerPos, GameContext& ctx) override;  // T8
};
