// enemy_spawner.cpp —— EnemySpawner 实现（T3：边缘出生 + 安全距离；构成/倍率 T8）
#include "enemy_spawner.h"

#include "constants.h"
#include "enemy_crawler.h"
#include "raymath.h"  // Vector2Distance

namespace {

// 在 [min, max] 区间取均匀随机数（基于 raylib 随机源）
float randomRange(float min, float max) {
    const float t = static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f;
    return min + t * (max - min);
}

}  // namespace

Vector2 EnemySpawner::pickSpawnPos(const Vector2& playerPos) const {
    using namespace constants::arena;
    using constants::enemy::kSpawnMinPlayerDist;
    using constants::enemy::kSpawnOutsideDepth;
    // 墙外出生带：距墙外缘 2~36px 的环形区域（开发者确认：敌人从竞技场外入场，
    // 直线追踪玩家并翻越围墙；正式入场规则仍可在 T8 波次阶段再调）
    const float nearDist = 2.0f;

    for (int attempt = 0; attempt < 30; ++attempt) {
        const int side = GetRandomValue(0, 3);  // 上/下/左/右四条边外随机
        const float t = randomRange(0.0f, 1.0f);                      // 沿边位置
        const float depth = randomRange(nearDist, kSpawnOutsideDepth); // 离墙深度
        Vector2 p{};
        switch (side) {
            case 0: p = {t * kWidth, -depth}; break;                    // 上墙外
            case 1: p = {t * kWidth, kHeight + depth}; break;           // 下墙外
            case 2: p = {-depth, t * kHeight}; break;                   // 左墙外
            default: p = {kWidth + depth, t * kHeight}; break;          // 右墙外
        }
        if (Vector2Distance(p, playerPos) >= kSpawnMinPlayerDist) return p;  // 保证不骑脸出生
    }

    // 重试耗尽兜底：取距玩家最远的场外角（距中心 1700px+，必然满足安全距离）
    const Vector2 corners[4] = {
        {-kSpawnOutsideDepth, -kSpawnOutsideDepth},
        {kWidth + kSpawnOutsideDepth, -kSpawnOutsideDepth},
        {-kSpawnOutsideDepth, kHeight + kSpawnOutsideDepth},
        {kWidth + kSpawnOutsideDepth, kHeight + kSpawnOutsideDepth},
    };
    Vector2 best = corners[0];
    float bestDist = Vector2Distance(best, playerPos);
    for (const Vector2& c : corners) {
        const float d = Vector2Distance(c, playerPos);
        if (d > bestDist) {
            bestDist = d;
            best = c;
        }
    }
    return best;
}

std::vector<std::unique_ptr<Enemy>> EnemySpawner::spawnWave(int count, const Vector2& playerPos) {
    std::vector<std::unique_ptr<Enemy>> out;
    out.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        out.push_back(std::make_unique<Crawler>(pickSpawnPos(playerPos)));
    }
    return out;
}