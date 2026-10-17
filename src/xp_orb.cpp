// xp_orb.cpp —— 经验点实现（T5：掉落视觉反馈；拾取 T7）
#include "xp_orb.h"

#include "constants.h"

XPOrb::XPOrb(const Vector2& pos, float value) : pos_(pos), value_(value) {}

void XPOrb::draw() const {
    // 以圆心为中心绘制边长 kSize 的实心方块（绿色，与敌人红/子弹黄区分）
    const float half = constants::xp_orb::kSize * 0.5f;
    const Vector2 topLeft{pos_.x - half, pos_.y - half};
    const Vector2 size{constants::xp_orb::kSize, constants::xp_orb::kSize};
    DrawRectangleV(topLeft, size, constants::xp_orb::kColor);
}