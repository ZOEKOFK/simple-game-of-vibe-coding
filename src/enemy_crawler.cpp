// enemy_crawler.cpp —— 爬行者实现（T3：追踪+分离力+接触标记；T5：接触数值/统一碰撞判定）
#include "enemy_crawler.h"

#include "collision.h"  // T5：统一圆-圆检测
#include "constants.h"
#include "game.h"     // GameContext 完整定义（分离力需遍历敌人列表）
#include "raymath.h"  // Vector2Length（raylib.h 未包含 raymath，需显式引入）

Crawler::Crawler(const Vector2& spawnPos) {
    pos_ = spawnPos;
    radius_ = constants::enemy::kCrawlerRadius;
    hp_ = maxHp_ = constants::enemy::kCrawlerHp;
    moveSpeed_ = constants::enemy::kCrawlerSpeed;
    contactDamage_ = constants::enemy::kCrawlerContactDamage;   // T5：接触伤害 8
    attackInterval_ = constants::enemy::kCrawlerAttackCooldown; // T5：攻击冷却 0.8s
    xpValue_ = constants::enemy::kCrawlerXpValue;               // T5：经验 5（击杀掉落）
}

void Crawler::decide(float dt, const Vector2& playerPos, GameContext& ctx) {
    // 1) 追踪：朝玩家方向
    const Vector2 toPlayer{playerPos.x - pos_.x, playerPos.y - pos_.y};
    const float distToPlayer = Vector2Length(toPlayer);
    Vector2 move{};
    if (distToPlayer > 0.0001f) {
        move = Vector2{toPlayer.x / distToPlayer, toPlayer.y / distToPlayer};
    }

    // 2) 分离力：同族过近时互相推离——距离越小推力越大，群体拥挤时不叠成一点
    if (ctx.enemies != nullptr) {
        Vector2 separation{};
        for (const auto& other : *ctx.enemies) {
            if (!other || other.get() == this) continue;  // 跳过自己与已移除的空位
            const Vector2 diff{pos_.x - other->pos().x, pos_.y - other->pos().y};
            const float d = Vector2Length(diff);
            if (d < constants::enemy::kSeparationRadius && d > 0.0001f) {
                const float push = (constants::enemy::kSeparationRadius - d) /
                                   constants::enemy::kSeparationRadius;
                separation.x += (diff.x / d) * push;
                separation.y += (diff.y / d) * push;
            }
        }
        move.x += separation.x * constants::enemy::kSeparationStrength;
        move.y += separation.y * constants::enemy::kSeparationStrength;
    }

    // 3) 归一化后按恒定移速移动：分离力只改变方向、不改变速度
    const float len = Vector2Length(move);
    if (len > 0.0001f) {
        move.x /= len;
        move.y /= len;
    }
    pos_.x += move.x * moveSpeed_ * dt;
    pos_.y += move.y * moveSpeed_ * dt;
    // 注意：此处不做竞技场钳制——敌人出生在墙外（开发者确认的入场方式），
    // 直线追踪玩家翻越围墙入场；通用边界约束在后续任务按需处理。

    // 4) 接触标记：统一走 collision 圆-圆检测（免开方；接触伤害由基类主调结算）
    touchingPlayer_ = ctx.player != nullptr &&
                      collision::circlesOverlap(pos_, radius_,
                                                ctx.player->pos(),
                                                ctx.player->radius());
}

void Crawler::draw() const {
    // 红色圆形 + 描边，与玩家蓝色明显区分；接触中时描边变亮黄（可观察的接触标记）
    DrawCircleV(pos_, radius_, constants::enemy::kCrawlerColor);
    DrawCircleLines(static_cast<int>(pos_.x), static_cast<int>(pos_.y), radius_,
                    touchingPlayer_ ? constants::enemy::kContactColor
                                    : constants::enemy::kCrawlerOutlineColor);
}