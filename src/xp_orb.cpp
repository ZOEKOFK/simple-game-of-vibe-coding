// xp_orb.cpp —— 经验点实现（T5：掉落视觉反馈；拾取 T7）
#include "xp_orb.h"

#include "assets.h"    // T12：绿色星形宝石贴图替代几何方块
#include "constants.h"

XPOrb::XPOrb(const Vector2& pos, float value) : pos_(pos), value_(value) {}

void XPOrb::draw() const {
    // T12：绿色星形宝石贴图替代实心方块——尺寸≈2×边长（视觉略大于拾取判定）
    assets::drawCentered(assets::xpGem(), pos_, constants::xp_orb::kSize * 2.0f,
                         0.0f, constants::xp_orb::kColor);
}