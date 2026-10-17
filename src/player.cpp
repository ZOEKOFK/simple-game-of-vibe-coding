// player.cpp —— Player 实现（T2：移动+自绘；T4：初始手枪与武器驱动；T5：受击/无敌帧/击退；T7 装备）
#include "player.h"

#include <cmath>
#include <memory>

#include "arena.h"
#include "constants.h"
#include "raymath.h"        // Vector2Length（raylib.h 未包含 raymath，需显式引入）
#include "weapon_pistol.h"  // 初始武器：手枪

Player::Player() {
    // 开局初始装备：手枪（docs/01 FR-05；本局武器列表随升级扩充）
    weapons_.push_back(std::make_unique<Pistol>(&stats_));
}

void Player::updateWeapons(float dt, GameContext& ctx) {
    // 逐武器驱动：索敌/冷却/开火逻辑封装在 Weapon 基类主流程中
    for (auto& w : weapons_) {
        w->update(dt, ctx, *this);
    }
}

const Vector2& Player::pos() const { return pos_; }  // 供镜头跟随等只读查询
float Player::radius() const { return constants::player::kRadius; }
int   Player::maxHp() const {  // 最大生命 = 基础值 + Stats 生命加成（加成词条 T7）
    return static_cast<int>(constants::player::kMaxHp + stats_.extraMaxHp);
}
int   Player::hp() const { return static_cast<int>(hp_); }  // 取整显示（伤害均为整值）

void Player::takeDamage(float amount, Vector2 fromPos) {
    if (invincibleTimer_ > 0.0f) return;  // 无敌帧内免伤（docs/03 4.1：0.6s）

    // 实际伤害 = 原始伤害 × 受伤倍率（Stats 默认 1.0；减伤词条 T7 生效）
    hp_ -= amount * stats_.damageTakenMult;
    if (hp_ < 0.0f) hp_ = 0.0f;  // HP 锁 0：死亡逻辑留待 T10，本任务仅锁定为 0

    invincibleTimer_ = constants::player::kInvincibleDuration;  // 进入无敌帧

    // 击退：沿"攻击者 → 玩家"方向施加初速（fromPos 为攻击来源），随后逐帧衰减
    Vector2 dir{pos_.x - fromPos.x, pos_.y - fromPos.y};
    const float len = Vector2Length(dir);
    if (len > 0.0001f) {
        dir.x /= len;
        dir.y /= len;
    } else {
        dir = Vector2{1.0f, 0.0f};  // 与攻击来源重合时兜底朝右
    }
    knockbackVel_ = Vector2{dir.x * constants::player::kKnockbackForce,
                            dir.y * constants::player::kKnockbackForce};
}

void Player::update(float dt, Vector2 inputDir) {
    // 对角归一化：方向向量先归一化再乘移速，保证斜向不加速
    const float len = Vector2Length(inputDir);
    if (len > 0.0001f) {
        inputDir.x /= len;
        inputDir.y /= len;
        lastMoveDir_ = inputDir;  // 静止时保持最后一次朝向
    }

    // 实际移速 = 基础移速 × Stats 移速倍率（默认倍率 1.0，词条 T7 接入）
    const float speed = constants::player::kMoveSpeed * stats_.moveSpeedMult;
    pos_.x += inputDir.x * speed * dt;
    pos_.y += inputDir.y * speed * dt;

    // 边界钳制：含碰撞半径，玩家不可嵌入围墙
    pos_ = clampToArena(pos_, constants::player::kRadius);

    // 无敌帧计时递减（T5：受击后 0.6s 内免伤；draw 中闪烁提示）
    if (invincibleTimer_ > 0.0f) invincibleTimer_ -= dt;

    // 击退衰减（T5）：受击瞬间获得初速，此后指数衰减并叠加到位移上，
    // 形成"被拍开"手感；总位移 ≈ 初速 / 衰减系数 = 240/10 = 24px
    const float decay = std::exp(-constants::player::kKnockbackDecay * dt);
    knockbackVel_.x *= decay;
    knockbackVel_.y *= decay;
    pos_.x += knockbackVel_.x * dt;
    pos_.y += knockbackVel_.y * dt;

    // 击退位移后再次钳制，防止被撞出墙外
    pos_ = clampToArena(pos_, constants::player::kRadius);
}

void Player::draw() const {
    // 无敌帧闪烁（T5）：每 0.2s 隐藏半周期，视觉提示"无敌中"
    if (invincibleTimer_ > 0.0f && std::fmod(invincibleTimer_, 0.2f) < 0.1f) return;

    const float r = constants::player::kRadius;
    // 身体：蓝色圆形 + 深色描边（T2 临时几何形象，后续任务细化）
    DrawCircleV(pos_, r, BLUE);
    DrawCircleLines(static_cast<int>(pos_.x), static_cast<int>(pos_.y), r, DARKBLUE);
    // 朝向指示线：从圆边缘沿朝向伸出
    const Vector2 lineStart{pos_.x + lastMoveDir_.x * (r - 4.0f),
                            pos_.y + lastMoveDir_.y * (r - 4.0f)};
    const Vector2 lineEnd{pos_.x + lastMoveDir_.x * (r + 12.0f),
                          pos_.y + lastMoveDir_.y * (r + 12.0f)};
    DrawLineEx(lineStart, lineEnd, 3.0f, YELLOW);
}