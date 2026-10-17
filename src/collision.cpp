// collision.cpp —— 碰撞工具实现（T5：圆-圆检测；T11：空间哈希网格）
#include "collision.h"

#include <cmath>        // std::floor

#include "constants.h"  // kCellSize / kMaxEntityRadius
#include "enemy.h"      // 邻域查询返回实体指针（调用方持有容器）
#include "projectile.h"
#include "xp_orb.h"

namespace collision {

bool circlesOverlap(const Vector2& aPos, float aRadius,
                    const Vector2& bPos, float bRadius) {
    // 免开方写法：比较平方距离与半径和平方，等价于求圆心距后开方比对
    const float sumR = aRadius + bRadius;
    const float dx = aPos.x - bPos.x;
    const float dy = aPos.y - bPos.y;
    return dx * dx + dy * dy <= sumR * sumR;
}

// ============================================================
// 空间哈希网格（T11）
// 键：单元索引 (ix, iy) 打包成 64 位（ix 高 32 位 / iy 低 32 位）；
//     世界坐标可为负（敌人墙外出生），floor 取整保证负数索引稳定。
// ============================================================

void SpatialGrid::clear() {
    enemyCells_.clear();
    bulletCells_.clear();
    orbCells_.clear();
    candidates_ = 0;
}

long long SpatialGrid::cellKey(float x, float y) const {
    using namespace constants::collision;
    const long long ix = static_cast<long long>(std::floor(x / static_cast<float>(kCellSize)));
    const long long iy = static_cast<long long>(std::floor(y / static_cast<float>(kCellSize)));
    return (ix << 32) ^ (static_cast<unsigned long long>(iy) & 0xFFFFFFFFULL);
}

void SpatialGrid::addEnemy(Enemy* e) {
    enemyCells_[cellKey(e->pos().x, e->pos().y)].push_back(e);
}

void SpatialGrid::addBullet(Projectile* p) {
    bulletCells_[cellKey(p->pos().x, p->pos().y)].push_back(p);
}

void SpatialGrid::addOrb(XPOrb* o) {
    orbCells_[cellKey(o->pos().x, o->pos().y)].push_back(o);
}

void SpatialGrid::queryEnemies(const Vector2& pos, float radius,
                               std::vector<Enemy*>& out) const {
    using namespace constants::collision;
    const float ext = radius + kMaxEntityRadius;
    const long long minIx = static_cast<long long>(std::floor((pos.x - ext) / kCellSize));
    const long long maxIx = static_cast<long long>(std::floor((pos.x + ext) / kCellSize));
    const long long minIy = static_cast<long long>(std::floor((pos.y - ext) / kCellSize));
    const long long maxIy = static_cast<long long>(std::floor((pos.y + ext) / kCellSize));
    for (long long ix = minIx; ix <= maxIx; ++ix) {
        for (long long iy = minIy; iy <= maxIy; ++iy) {
            const long long key = (ix << 32) ^ (static_cast<unsigned long long>(iy) & 0xFFFFFFFFULL);
            const auto it = enemyCells_.find(key);
            if (it != enemyCells_.end()) {
                candidates_ += static_cast<int>(it->second.size());
                out.insert(out.end(), it->second.begin(), it->second.end());
            }
        }
    }
}

void SpatialGrid::queryBullets(const Vector2& pos, float radius,
                               std::vector<Projectile*>& out) const {
    using namespace constants::collision;
    const float ext = radius + kMaxEntityRadius;
    const long long minIx = static_cast<long long>(std::floor((pos.x - ext) / kCellSize));
    const long long maxIx = static_cast<long long>(std::floor((pos.x + ext) / kCellSize));
    const long long minIy = static_cast<long long>(std::floor((pos.y - ext) / kCellSize));
    const long long maxIy = static_cast<long long>(std::floor((pos.y + ext) / kCellSize));
    for (long long ix = minIx; ix <= maxIx; ++ix) {
        for (long long iy = minIy; iy <= maxIy; ++iy) {
            const long long key = (ix << 32) ^ (static_cast<unsigned long long>(iy) & 0xFFFFFFFFULL);
            const auto it = bulletCells_.find(key);
            if (it != bulletCells_.end()) {
                candidates_ += static_cast<int>(it->second.size());
                out.insert(out.end(), it->second.begin(), it->second.end());
            }
        }
    }
}

void SpatialGrid::queryOrbs(const Vector2& pos, float radius,
                            std::vector<XPOrb*>& out) const {
    using namespace constants::collision;
    const float ext = radius + kMaxEntityRadius;
    const long long minIx = static_cast<long long>(std::floor((pos.x - ext) / kCellSize));
    const long long maxIx = static_cast<long long>(std::floor((pos.x + ext) / kCellSize));
    const long long minIy = static_cast<long long>(std::floor((pos.y - ext) / kCellSize));
    const long long maxIy = static_cast<long long>(std::floor((pos.y + ext) / kCellSize));
    for (long long ix = minIx; ix <= maxIx; ++ix) {
        for (long long iy = minIy; iy <= maxIy; ++iy) {
            const long long key = (ix << 32) ^ (static_cast<unsigned long long>(iy) & 0xFFFFFFFFULL);
            const auto it = orbCells_.find(key);
            if (it != orbCells_.end()) {
                candidates_ += static_cast<int>(it->second.size());
                out.insert(out.end(), it->second.begin(), it->second.end());
            }
        }
    }
}

}  // namespace collision
