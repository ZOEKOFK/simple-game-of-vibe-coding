#pragma once
// ============================================================
// collision.h —— 碰撞工具
// T5：圆-圆相交检测工具函数（免开方）；
// T11：空间哈希网格分区与邻域查询在此扩展（docs/03 2.8 渐进替换方案）。
// ============================================================
#include "raylib.h"  // Vector2

namespace collision {

// 圆-圆相交判定：半径和平方 与 圆心距平方 比较（避免开方，docs/04 T5 要求）
bool circlesOverlap(const Vector2& aPos, float aRadius,
                    const Vector2& bPos, float bRadius);

}  // namespace collision