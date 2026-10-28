// arena.cpp —— 竞技场实现（T2：边界钳制与 地面/网格/围墙 绘制）
#include "arena.h"

#include <algorithm>

#include "constants.h"

Vector2 clampToArena(const Vector2& pos, float radius) {
    using namespace constants::arena;
    // 可活动范围 = 墙内缘再内收一个碰撞半径
    const float minX = kWallThickness + radius;
    const float maxX = kWidth - kWallThickness - radius;
    const float minY = kWallThickness + radius;
    const float maxY = kHeight - kWallThickness - radius;

    Vector2 clamped = pos;
    clamped.x = std::clamp(clamped.x, minX, maxX);
    clamped.y = std::clamp(clamped.y, minY, maxY);
    return clamped;
}

void drawArena() {
    using namespace constants::arena;
    // 1. 内场地面（深色，与场外底色区分）
    DrawRectangleRec({0.0f, 0.0f, kWidth, kHeight}, kGroundColor);
    // 2. 网格线：横竖各一组（间距 kGridSpacing），提供移动与距离参照
    for (float x = kGridSpacing; x < kWidth; x += kGridSpacing) {
        DrawLineEx({x, 0.0f}, {x, kHeight}, kGridLineThickness, kGridColor);
    }
    for (float y = kGridSpacing; y < kHeight; y += kGridSpacing) {
        DrawLineEx({0.0f, y}, {kWidth, y}, kGridLineThickness, kGridColor);
    }
    // 3. 四周围墙（厚度 kWallThickness，醒目橙红色）
    DrawRectangleRec({0.0f, 0.0f, kWidth, kWallThickness}, kWallColor);  // 上
    DrawRectangleRec({0.0f, kHeight - kWallThickness, kWidth,
                      kWallThickness}, kWallColor);                     // 下
    DrawRectangleRec({0.0f, kWallThickness, kWallThickness,
                      kHeight - 2.0f * kWallThickness}, kWallColor);    // 左
    DrawRectangleRec({kWidth - kWallThickness, kWallThickness,
                      kWallThickness, kHeight - 2.0f * kWallThickness},
                     kWallColor);                                       // 右
    // 4. 墙外缘深色层 + 墙身砖缝线（T13：增强墙体厚度与警戒质感，纯视觉不动物理）
    constexpr float kOuterRim = 8.0f;     // 墙外缘深色层厚度（视觉分层）
    constexpr float kBrickStep = 24.0f;   // 砖缝间距（px）
    DrawRectangleRec({0.0f, 0.0f, kWidth, kOuterRim}, kWallOuter);                // 上外缘
    DrawRectangleRec({0.0f, kHeight - kOuterRim, kWidth, kOuterRim}, kWallOuter); // 下外缘
    DrawRectangleRec({0.0f, 0.0f, kOuterRim, kHeight}, kWallOuter);               // 左外缘
    DrawRectangleRec({kWidth - kOuterRim, 0.0f, kOuterRim, kHeight}, kWallOuter); // 右外缘
    for (float x = kBrickStep; x < kWidth; x += kBrickStep) {                     // 上/下墙竖缝
        DrawLineEx({x, kOuterRim}, {x, kWallThickness}, 1.5f, kWallSeam);
        DrawLineEx({x, kHeight - kWallThickness}, {x, kHeight - kOuterRim}, 1.5f, kWallSeam);
    }
    for (float y = kWallThickness + kBrickStep; y < kHeight - kWallThickness;
         y += kBrickStep) {                                                       // 左/右墙横缝
        DrawLineEx({kOuterRim, y}, {kWallThickness, y}, 1.5f, kWallSeam);
        DrawLineEx({kWidth - kWallThickness, y}, {kWidth - kOuterRim, y}, 1.5f, kWallSeam);
    }
    // 5. 地面贴墙一圈内缘阴影带（T13：地面→墙过渡加深，俯视纵深感；纯视觉）
    constexpr float kInnerShade = 6.0f;
    const Color shade{0, 0, 0, 42};
    DrawRectangleRec({kWallThickness, kWallThickness, kWidth - 2.0f * kWallThickness,
                      kInnerShade}, shade);
    DrawRectangleRec({kWallThickness, kHeight - kWallThickness - kInnerShade,
                      kWidth - 2.0f * kWallThickness, kInnerShade}, shade);
    DrawRectangleRec({kWallThickness, kWallThickness, kInnerShade,
                      kHeight - 2.0f * kWallThickness}, shade);
    DrawRectangleRec({kWidth - kWallThickness - kInnerShade, kWallThickness, kInnerShade,
                      kHeight - 2.0f * kWallThickness}, shade);
    // 6. 墙内缘高亮描边，强化可活动边界（保持最后绘制，轮廓最清晰）
    DrawRectangleLinesEx({kWallThickness, kWallThickness,
                          kWidth - 2.0f * kWallThickness,
                          kHeight - 2.0f * kWallThickness},
                         3.0f, kWallEdgeColor);
}