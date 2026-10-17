#pragma once
// ============================================================
// collision.h —— 碰撞工具
// T5：圆-圆相交检测工具函数（免开方）；
// T11：空间哈希网格分区与邻域查询（docs/03 2.8 渐进替换方案落地）。
//       动态实体（敌人/子弹/经验点）按中心登记到 160×160 单元，
//       查询时取 (pos ± (radius + kMaxEntityRadius)) 覆盖的单元集，
//       精确相交仍由 circlesOverlap / 距离检查判定 —— 与朴素双循环行为等价。
// ============================================================
#include <unordered_map>
#include <vector>

#include "raylib.h"  // Vector2

class Enemy;      // 前向声明：网格仅存指针并做邻域查询，不持有类型
class Projectile;
class XPOrb;

namespace collision {

// 圆-圆相交判定：半径和平方 与 圆心距平方 比较（避免开方，docs/04 T5 要求）
bool circlesOverlap(const Vector2& aPos, float aRadius,
                    const Vector2& bPos, float bRadius);

// 空间哈希网格（T11）：每帧 clear() 后按实体中心重新登记，
// 查询返回"邻域候选"（精确相交由调用方完成），候选数计入性能日志。
class SpatialGrid {
public:
    void clear();                       // 帧内重建：清空全部桶与候选计数
    void addEnemy(Enemy* e);
    void addBullet(Projectile* p);
    void addOrb(XPOrb* o);

    // 邻域查询：收集与 pos 距离 ≤ radius + kMaxEntityRadius 的候选实体
    void queryEnemies(const Vector2& pos, float radius,
                      std::vector<Enemy*>& out) const;
    void queryBullets(const Vector2& pos, float radius,
                      std::vector<Projectile*>& out) const;
    void queryOrbs(const Vector2& pos, float radius,
                   std::vector<XPOrb*>& out) const;

    // T11 性能日志：本帧（自上次 clear）邻域查询返回的候选总数（比较次数下界）
    int candidates() const { return candidates_; }

private:
    long long cellKey(float x, float y) const;   // 世界坐标 → 单元哈希键（可负坐标安全）

    std::unordered_map<long long, std::vector<Enemy*>> enemyCells_;
    std::unordered_map<long long, std::vector<Projectile*>> bulletCells_;
    std::unordered_map<long long, std::vector<XPOrb*>> orbCells_;
    mutable int candidates_ = 0;
};

}  // namespace collision
