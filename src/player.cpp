// player.cpp —— Player 实现（T2：移动+自绘；T4：初始手枪与武器驱动；T5 受击、T7 装备）
#include "player.h"

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
    // 无敌帧/击退衰减：T5 实现
}

void Player::draw() const {
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