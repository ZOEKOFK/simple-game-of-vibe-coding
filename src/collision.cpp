// collision.cpp —— 碰撞工具实现（T5：圆-圆检测）
#include "collision.h"

namespace collision {

bool circlesOverlap(const Vector2& aPos, float aRadius,
                    const Vector2& bPos, float bRadius) {
    // 免开方写法：比较平方距离与半径和平方，等价于求圆心距后开方比对
    const float sumR = aRadius + bRadius;
    const float dx = aPos.x - bPos.x;
    const float dy = aPos.y - bPos.y;
    return dx * dx + dy * dy <= sumR * sumR;
}

}  // namespace collision