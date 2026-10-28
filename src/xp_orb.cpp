// xp_orb.cpp —— 经验点实现（T5：掉落视觉反馈；拾取 T7；T13 程序化六边形宝石）
#include "xp_orb.h"

#include <cmath>   // std::sin / std::cos（T12 脉动；T13 宝石切面/自转）

#include "constants.h"
#include "raymath.h"  // PI / Vector2（几何运算）

XPOrb::XPOrb(const Vector2& pos, float value) : pos_(pos), value_(value) {}

void XPOrb::draw() const {
    // T13：经验宝石改程序化六边形宝石（T12 贴图已废弃）——
    //      青绿渐变面 + 白描边 + 顶部高光 + 呼吸光晕 + 慢速自转，
    //      与敌弹红紫、玩家弹金橙明显区分。视觉尺寸沿用 kVisualSize(30)×脉动，
    //      拾取/吸取判定不受绘制影响。
    const float phase = (pos_.x + pos_.y) * 0.1f;  // 位置错相：避免全场同步缩放
    const float pulse = 1.0f +
        constants::xp_orb::kPulseAmp *
            std::sin(GetTime() * constants::xp_orb::kPulseSpeed + phase);
    const float R = constants::xp_orb::kVisualSize * 0.5f * pulse;  // 外接圆半径 ≈15×pulse
    const float spin = std::fmod(constants::xp_orb::kSpinSpeed * GetTime(), 2.0f * PI);

    // 呼吸光晕（底层，随脉动一起缩放）
    DrawCircleV(pos_, R * 1.45f, constants::xp_orb::kGemGlow);

    // 宝石面：深青绿大六边形 + 亮青绿小六边形向左上偏移（渐变切片感）
    DrawPoly(pos_, 6, R, spin * (180.0f / PI), constants::xp_orb::kGemDark);
    const Vector2 hiPos{pos_.x - R * 0.12f, pos_.y - R * 0.14f};
    DrawPoly(hiPos, 6, R * 0.60f, spin * (180.0f / PI), constants::xp_orb::kGemLight);

    // 顶部高光小菱形（随宝石同转，固定在切面上角）
    const float hx = pos_.x + (std::cos(spin) * R * 0.38f - std::sin(spin) * (-R * 0.38f));
    const float hy = pos_.y + (std::sin(spin) * R * 0.38f + std::cos(spin) * (-R * 0.38f));
    DrawRectanglePro(Rectangle{hx, hy, 6.0f, 6.0f}, Vector2{3.0f, 3.0f},
                     spin * (180.0f / PI) + 45.0f, constants::xp_orb::kGemEdge);

    // 白描边：六条边逐段连线（沿外接圆切角，与宝石面同步旋转）
    for (int i = 0; i < 6; ++i) {
        const float th = spin + i * (PI / 3.0f);
        const float th2 = th + PI / 3.0f;
        const Vector2 p1{pos_.x + std::cos(th) * R, pos_.y + std::sin(th) * R};
        const Vector2 p2{pos_.x + std::cos(th2) * R, pos_.y + std::sin(th2) * R};
        DrawLineEx(p1, p2, 2.0f, constants::xp_orb::kGemEdge);
    }
}