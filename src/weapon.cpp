// weapon.cpp —— Weapon 实现（T4：索敌主流程；T9：工厂注册表与再选升级）
#include "weapon.h"

#include "constants.h"  // T9：再选升级伤害增量
#include "enemy.h"      // Enemy 位置/半径查询
#include "game.h"       // GameContext 完整定义：敌人遍历 + 子弹容器
#include "player.h"     // Player 位置查询
#include "raymath.h"    // Vector2Length（基类默认开火前置处理用）
#include "stats.h"      // Stats 攻速倍率（T7 词条联动）

namespace {

// 索敌：遍历敌人，取"距玩家最近且距离 ≤ 射程"者；无目标返回 nullptr
const Enemy* findNearestEnemyInRange(GameContext& ctx, const Vector2& playerPos,
                                     float range) {
    if (ctx.enemies == nullptr) return nullptr;
    const Enemy* found = nullptr;
    float bestDistSq = range * range;
    for (const auto& e : *ctx.enemies) {
        if (!e) continue;
        const float dx = e->pos().x - playerPos.x;
        const float dy = e->pos().y - playerPos.y;
        const float distSq = dx * dx + dy * dy;
        if (distSq <= bestDistSq) {
            bestDistSq = distSq;
            found = e.get();
        }
    }
    return found;
}

}  // namespace

const std::string& Weapon::typeId() const { return typeId_; }

// ============================================================
// T9：工厂注册表与再选升级（新增武器不改旧文件的关键机制）
// ============================================================
std::unordered_map<std::string, Weapon::Factory>& Weapon::registry() {
    static std::unordered_map<std::string, Weapon::Factory> reg;
    return reg;
}

void Weapon::registerWeapon(const std::string& id, Weapon::Factory f) {
    registry().emplace(id, std::move(f));  // 重复注册安全忽略（id 全局唯一约定）
}

std::unique_ptr<Weapon> Weapon::create(const std::string& id, const Stats* stats) {
    auto& reg = registry();
    const auto it = reg.find(id);
    if (it == reg.end()) return nullptr;  // 未注册 id：返回空（Player::giveWeapon 丢弃）
    return it->second(stats);
}

void Weapon::upgradePick() {
    // 装备词条再选到已有武器：伤害按武器差异化成长（手枪 +6 / 霰弹每弹丸 +3 / 飞刃 +4），
    // 保证各武器复选收益统一在 ≈+25%（docs/05 §4.1 平衡自查结论）
    baseDamage_ += upgradeDamageAmount();
}

float Weapon::upgradeDamageAmount() const {
    // 默认（单发类）= 手枪增量；多弹丸/多段类在子类覆写为更低值
    return constants::weapon::kWeaponUpgradeDamage;
}

void Weapon::draw(const Player& player) const {
    // 发射型武器无持续本体；环绕飞刃覆写本接口自绘
    (void)player;
}

Weapon::Weapon(float fireInterval, float damage, float range) {
    fireInterval_ = fireInterval;
    baseDamage_ = damage;
    range_ = range;
}

void Weapon::update(float dt, GameContext& ctx, const Player& player) {
    // 1) 冷却计时：距上次开火已过时间
    if (fireTimer_ > 0.0f) fireTimer_ -= dt;

    // 2) 索敌：射程内最近敌人（目标死亡/离开射程后下一帧自动更换）
    const Enemy* target = findNearestEnemyInRange(ctx, player.pos(), range_);
    if (target == nullptr) return;  // 无目标不开火

    // 3) 到点开火：具体开火模式（单发/散射/持续）由子类覆写 fire()/update() 实现
    if (fireTimer_ <= 0.0f) {
        fire(ctx, player, target->pos());
        // T7 词条联动：实际间隔 = 基础间隔 ÷ 攻速倍率（急速/神速生效；环绕飞刃无间隔不参与）
        fireTimer_ = fireInterval_ /
                     (stats_ != nullptr ? stats_->attackSpeedMult : 1.0f);
    }
}

void Weapon::fire(GameContext& ctx, const Player& player, const Vector2& targetPos) {
    // 基类默认不开火；发射型子类（手枪/霰弹）必须覆写本方法
    (void)ctx;
    (void)player;
    (void)targetPos;
}