// wave_manager.cpp —— WaveManager 实现（T8：波次状态机 + 构成权重）
#include "wave_manager.h"

#include <algorithm>  // std::min
#include <memory>     // std::make_unique / std::move（保险，部分平台 unique_ptr 需显式）

#include "game.h"     // GameContext 完整定义（ctx.enemies / ctx.spawner / ctx.player）
#include "raylib.h"   // GetRandomValue 随机源

namespace {

// [0,1) 均匀随机数；GetRandomValue 上限 32767（raylib 约定，超出触发警告且结果不可靠）
float random01() {
    return static_cast<float>(GetRandomValue(0, 32767)) / 32767.0f;
}

}  // namespace

int   WaveManager::wave() const { return wave_; }
WaveManager::State WaveManager::state() const { return state_; }
float WaveManager::restTimeLeft() const { return restTimer_; }
int   WaveManager::remainingToSpawn() const { return spawnBatchLeft_; }
int   WaveManager::aliveLeft() const { return aliveLeft_; }

void WaveManager::reset() {
    // T10：重开局复位——第 1 波、3s 休整倒计时、刷怪批次与存活账本/蛮牛计数全部清零
    state_ = State::WAITING;
    wave_ = 1;
    restTimer_ = constants::wave::kRestTime;
    spawnBatchLeft_ = 0;
    batchTimer_ = 0.0f;
    aliveLeft_ = 0;
    brutesSpawned_ = 0;
}

int WaveManager::totalCountForWave(int n) const {
    return constants::wave::kBaseCount + constants::wave::kCountPerWave * (n - 1);
}

float WaveManager::hpMultiplierForWave(int n) const {
    return constants::wave::kHpMultBase + constants::wave::kHpMultPerWave * (n - 1);
}

void WaveManager::startNextWave() {
    // WAITING 倒计时归零 → 进入 SPAWNING；第一批立即投放（batchTimer_ 置 0），
    // 之后每 0.5s 一小批（kBatchSize 只），避免一帧全刷造成画面爆发
    state_ = State::SPAWNING;
    spawnBatchLeft_ = totalCountForWave(wave_);
    batchTimer_ = 0.0f;
    brutesSpawned_ = 0;
}

void WaveManager::notifyEnemyDied() {
    // 本波账本递减：F1 调试刷出的怪死亡时若本波已无存活账（0），钳制不动；
    // 清场判定同时要求场上容器为空（含 F1 残留），双条件保证 F1 不影响波次正确性
    if (aliveLeft_ > 0) --aliveLeft_;
}

EnemyType WaveManager::rollType() {
    using namespace constants::wave;
    // 1~4 波纯爬行者；5~7 波 80/20 混入喷吐者；8+ 波 70/22/8 且蛮牛每波 ≤ 2 只
    if (wave_ < kSpitterFirstWave) return EnemyType::Crawler;

    const float r = random01();
    if (wave_ < kBruteFirstWave) {
        return r < kSpitterWeightMid ? EnemyType::Spitter
                                     : EnemyType::Crawler;
    }
    if (r < kCrawlerWeightLate) return EnemyType::Crawler;
    if (r < kCrawlerWeightLate + kSpitterWeightLate) return EnemyType::Spitter;
    if (brutesSpawned_ >= kBruteMaxPerWave) return EnemyType::Crawler;  // 超上限退回
    ++brutesSpawned_;
    return EnemyType::Brute;
}

void WaveManager::update(float dt, GameContext& ctx) {
    switch (state_) {
        case State::WAITING: {
            // 波间休整倒计时：HUD 显示"下一波 X: 3, 2, 1"（数值 = restTimeLeft）
            restTimer_ -= dt;
            if (restTimer_ <= 0.0f) startNextWave();
            break;
        }
        case State::SPAWNING: {
            batchTimer_ -= dt;
            if (batchTimer_ > 0.0f) break;
            if (ctx.enemies == nullptr || ctx.spawner == nullptr) break;

            // 每 0.5s 投放一小批（kBatchSize 只），不足一批则一次放完
            batchTimer_ = constants::wave::kBatchInterval;
            const int batch = std::min(constants::wave::kBatchSize, spawnBatchLeft_);
            const float hpMult = hpMultiplierForWave(wave_);
            const Vector2 playerPos =
                ctx.player != nullptr ? ctx.player->pos()
                                      : Vector2{constants::player::kStartX,
                                                constants::player::kStartY};
            for (int i = 0; i < batch; ++i) {
                std::unique_ptr<Enemy> e = ctx.spawner->spawn(rollType(), playerPos, hpMult);
                if (e) {
                    ++aliveLeft_;  // 本波存活账本同步记账
                    ctx.enemies->push_back(std::move(e));
                }
            }
            spawnBatchLeft_ -= batch;
            if (spawnBatchLeft_ <= 0) state_ = State::FIGHTING;  // 本波投放完毕 → 清场阶段
            break;
        }
        case State::FIGHTING: {
            // 清场判定（双条件）：本波账本清零 且 场上容器为空（含 F1 调试残留）。
            // 敌人移除发生在 GameEngine 帧末，故检测到清场最快在下一帧，属正常一拍延迟。
            if (aliveLeft_ <= 0 && ctx.enemies != nullptr && ctx.enemies->empty()) {
                ++wave_;                                        // 波号 +1
                restTimer_ = constants::wave::kRestTime;        // 重新进入 3s 休整
                state_ = State::WAITING;
            }
            break;
        }
    }
}