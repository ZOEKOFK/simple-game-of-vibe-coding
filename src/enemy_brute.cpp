// enemy_brute.cpp —— 蛮牛实现（T8：蓄力冲锋状态机，AI 展示亮点）
#include "enemy_brute.h"

#include <cmath>  // sinf：蓄力闪烁

#include "collision.h"
#include "constants.h"
#include "game.h"    // GameContext 完整定义（接触判定经 ctx.player 结算）
#include "raymath.h" // Vector2Length

Brute::Brute(const Vector2& spawnPos) {
    pos_ = spawnPos;
    radius_ = constants::enemy::kBruteRadius;            // ×2 体型
    hp_ = maxHp_ = constants::enemy::kBruteHp;           // ×30 生命（波次倍率在出生后叠加）
    moveSpeed_ = constants::enemy::kBruteSpeed;          // ×0.7 移速
    contactDamage_ = constants::enemy::kBruteContactDamage; // ×3 接触伤害
    attackInterval_ = constants::enemy::kBruteChargeCooldown; // 接触结算同样走冲锋冷却节奏（docs/03 4.3）
    xpValue_ = constants::enemy::kBruteXpValue;          // ×10 经验
}

void Brute::decide(float dt, const Vector2& playerPos, GameContext& ctx) {
    const Vector2 toPlayer{playerPos.x - pos_.x, playerPos.y - pos_.y};
    const float dist = Vector2Length(toPlayer);
    const Vector2 dirToPlayer =
        dist > 0.0001f ? Vector2{toPlayer.x / dist, toPlayer.y / dist} : Vector2{0.0f, 0.0f};

    switch (phase_) {
        case Phase::Chase: {
            // 普通追踪：慢速朝玩家逼近，进入蓄力触发距离且冷却就绪 → 开始蓄力
            pos_.x += dirToPlayer.x * moveSpeed_ * dt;
            pos_.y += dirToPlayer.y * moveSpeed_ * dt;
            if (cooldownTimer_ <= 0.0f &&
                dist <= constants::enemy::kBruteWindupTriggerDist) {
                phase_ = Phase::Windup;
                windupTimer_ = constants::enemy::kBruteWindupTime;
            }
            break;
        }
        case Phase::Windup: {
            // 蓄力 0.8s：原地钉住（可读预警窗口），持续锁定玩家方位作为冲刺方向——
            // 玩家必须在这 0.8s 内横向拉开角度，否则被 252px/s 冲锋撞上
            chargeDir_ = dirToPlayer;
            windupTimer_ -= dt;
            if (windupTimer_ <= 0.0f) {
                phase_ = Phase::Charge;
                chargeTimer_ = constants::enemy::kBruteChargeTime;
            }
            break;
        }
        case Phase::Charge: {
            // 冲锋 1s：沿冻结方向直线冲刺，速度×3（252px/s > 玩家 220px/s，跑不过只能预读闪避）
            const float chargeSpeed = moveSpeed_ * constants::enemy::kBruteChargeSpeedMult;
            pos_.x += chargeDir_.x * chargeSpeed * dt;
            pos_.y += chargeDir_.y * chargeSpeed * dt;
            chargeTimer_ -= dt;
            if (chargeTimer_ <= 0.0f) {
                phase_ = Phase::Cooldown;
                cooldownTimer_ = constants::enemy::kBruteChargeCooldown;
            }
            break;
        }
        case Phase::Cooldown: {
            // 疲惫期 5s：恢复普通追踪速度逼近
            pos_.x += dirToPlayer.x * moveSpeed_ * dt;
            pos_.y += dirToPlayer.y * moveSpeed_ * dt;
            cooldownTimer_ -= dt;
            if (cooldownTimer_ <= 0.0f) phase_ = Phase::Chase;
            break;
        }
    }

    // 接触标记：统一圆-圆检测（冲锋撞上即触发基类接触伤害结算 ×3）
    touchingPlayer_ = ctx.player != nullptr &&
                      collision::circlesOverlap(pos_, radius_,
                                                ctx.player->pos(),
                                                ctx.player->radius());
}

void Brute::draw() const {
    using constants::enemy::kBruteChargeColor;
    using constants::enemy::kBruteColor;
    using constants::enemy::kBruteWindupColor;
    using constants::enemy::kBruteWindupRingColor;

    Color body = kBruteColor;
    switch (phase_) {
        case Phase::Windup: {
            // 蓄力预警（AI 亮点）：主体深红→亮红渐变 + 高频闪烁 + 警戒环 + 冲刺方向箭头
            const float p = 1.0f - windupTimer_ / constants::enemy::kBruteWindupTime;  // 0→1
            const float blink = 0.5f + 0.5f * sinf(windupTimer_ * 45.0f);              // 高频闪
            const float t = p * 0.65f + blink * 0.35f;
            body.r = static_cast<unsigned char>(kBruteColor.r +
                       (kBruteWindupColor.r - kBruteColor.r) * t);
            body.g = static_cast<unsigned char>(kBruteColor.g +
                       (kBruteWindupColor.g - kBruteColor.g) * t);
            body.b = static_cast<unsigned char>(kBruteColor.b +
                       (kBruteWindupColor.b - kBruteColor.b) * t);
            // 警戒环：半径随蓄力进度扩张（60→84），透明度渐增
            const float ring = radius_ + 14.0f + 24.0f * p;
            DrawCircleLines(static_cast<int>(pos_.x), static_cast<int>(pos_.y), ring,
                            kBruteWindupRingColor);
            // 冲刺方向箭头：基点到尖端的粗线 + 两侧翼线（指向蓄力锁定方位）
            const float baseD = radius_ + 10.0f;
            const float tipD  = radius_ + 40.0f;
            const Vector2 base{pos_.x + chargeDir_.x * baseD, pos_.y + chargeDir_.y * baseD};
            const Vector2 tip {pos_.x + chargeDir_.x * tipD,  pos_.y + chargeDir_.y * tipD};
            const Vector2 perp{-chargeDir_.y, chargeDir_.x};
            DrawLineEx(Vector2{base.x + perp.x * 9.0f, base.y + perp.y * 9.0f}, tip, 3.0f,
                       kBruteWindupColor);
            DrawLineEx(base, tip, 4.0f, kBruteWindupColor);
            DrawLineEx(Vector2{base.x - perp.x * 9.0f, base.y - perp.y * 9.0f}, tip, 3.0f,
                       kBruteWindupColor);
            break;
        }
        case Phase::Charge: {
            // 冲锋：亮红 + 身后拖尾两圈渐变（速度感）
            body = kBruteChargeColor;
            const Color tail1{kBruteChargeColor.r, kBruteChargeColor.g, kBruteChargeColor.b, 90};
            const Color tail2{kBruteChargeColor.r, kBruteChargeColor.g, kBruteChargeColor.b, 45};
            DrawCircleV(Vector2{pos_.x - chargeDir_.x * 14.0f, pos_.y - chargeDir_.y * 14.0f},
                        radius_ * 0.8f, tail1);
            DrawCircleV(Vector2{pos_.x - chargeDir_.x * 26.0f, pos_.y - chargeDir_.y * 26.0f},
                        radius_ * 0.6f, tail2);
            break;
        }
        case Phase::Chase:
        default:
            body = kBruteColor;  // 追踪/疲惫：深红本体
            break;
    }
    DrawCircleV(pos_, radius_, body);
    DrawCircleLines(static_cast<int>(pos_.x), static_cast<int>(pos_.y), radius_,
                    constants::enemy::kBruteOutlineColor);
}