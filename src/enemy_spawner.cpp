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
    // 出生带 = 墙内缘内收"一个敌人半径 + 少量余量"，视觉上从场地边缘走向玩家
    const float inset = kWallThickness + constants::enemy::kCrawlerRadius + 8.0f;

    for (int attempt = 0; attempt < 30; ++attempt) {
        const int side = GetRandomValue(0, 3);  // 上/下/左/右四条边随机
        const float t = randomRange(0.0f, 1.0f);
        Vector2 p{};
        switch (side) {
            case 0: p = {inset + t * (kWidth - 2.0f * inset), inset}; break;                // 上边
            case 1: p = {inset + t * (kWidth - 2.0f * inset), kHeight - inset}; break;      // 下边
            case 2: p = {inset, inset + t * (kHeight - 2.0f * inset)}; break;               // 左边
            default: p = {kWidth - inset, inset + t * (kHeight - 2.0f * inset)}; break;     // 右边
        }
        if (Vector2Distance(p, playerPos) >= kSpawnMinPlayerDist) return p;  // 保证不骑脸出生
    }

    // 重试耗尽兜底：取距玩家最远的场角（距中心 1700px+，必然满足安全距离）
    const Vector2 corners[4] = {
        {inset, inset}, {kWidth - inset, inset},
        {inset, kHeight - inset}, {kWidth - inset, kHeight - inset},
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