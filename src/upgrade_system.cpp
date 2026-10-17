// upgrade_system.cpp —— UpgradeSystem 实现（T7：经验曲线/溢出/三选一抽取/词条应用）
#include "upgrade_system.h"

#include <algorithm>
#include <string>

#include "game.h"    // GameContext 完整定义（玩家武器列表：前置词条过滤）
#include "player.h"  // Player::statsMutable / heal
#include "raylib.h"  // GetRandomValue（伪随机抽取）

namespace {

// 玩家是否已持有某 typeId 武器（前置词条过滤：如"飞刃强化"需先有 orbital）
bool playerHasWeapon(const GameContext& ctx, const std::string& typeId) {
    if (ctx.player == nullptr) return false;
    const auto& ws = ctx.player->weapons();
    for (const auto& w : ws) {
        if (w && w->typeId() == typeId) return true;
    }
    return false;  // 未持有该武器（装备词条前置过滤：如"飞刃强化"需先有 orbiting 飞刃）
}

// 两段式抽取第一段：稀有度权重 普通 60 / 稀有 30 / 史诗 10（docs/03 4.6）
Rarity rollRarity() {
    const int r = GetRandomValue(1, 100);  // 闭区间 [1,100]
    if (r <= 60) return Rarity::Common;
    if (r <= 90) return Rarity::Rare;
    return Rarity::Epic;
}

}  // namespace

int UpgradeSystem::level() const { return level_; }
int UpgradeSystem::expInLevel() const { return exp_; }

void UpgradeSystem::addExp(int amount, GameContext& ctx) {
    (void)ctx;  // T7 无需外部协作；保留签名与设计文档接口一致（docs/03 2.6）
    if (amount <= 0) return;
    exp_ += amount;
    // 溢出保留：每攒满一级所需经验立即扣减并登记一次升级请求（可连升多级）
    while (exp_ >= expToNext()) {
        exp_ -= expToNext();
        ++level_;
        ++pendingUpgrades_;
    }
}

bool UpgradeSystem::consumePendingUpgrade() {
    if (pendingUpgrades_ > 0) --pendingUpgrades_;
    return pendingUpgrades_ > 0;  // 返回"是否仍有剩余"（决定是否留在 UPGRADE 重抽）
}

std::vector<Affix> UpgradeSystem::drawChoices(int count, GameContext& ctx) {
    // 候选池：剔除"前置武器不满足"的词条（T7 只有"飞刃强化"不可达，T9 解禁）
    std::vector<const Affix*> candidates;
    for (const auto& a : affixPool()) {
        if (!a.requiresWeapon.empty() && !playerHasWeapon(ctx, a.requiresWeapon)) continue;
        candidates.push_back(&a);
    }

    // 剔除上一轮已展示词条防连续重复；剩余不足 count 时放弃过滤（允许重复）
    std::vector<const Affix*> fresh;
    for (const Affix* a : candidates) {
        if (std::find(lastShown_.begin(), lastShown_.end(), a->name) == lastShown_.end()) {
            fresh.push_back(a);
        }
    }
    if (fresh.size() >= static_cast<size_t>(count)) candidates = fresh;

    // 两段式抽取：先抽稀有度，再在该稀有度子池内等概率抽词条；
    // 子池耗尽（如候选不足）则放宽为全候选等概率，保证总能凑足三选项
    std::vector<Affix> result;
    std::vector<const Affix*> bag = candidates;
    while (static_cast<int>(result.size()) < count && !bag.empty()) {
        const Rarity r = rollRarity();
        std::vector<const Affix*> sub;
        for (const Affix* a : bag) {
            if (a->rarity == r) sub.push_back(a);
        }
        if (sub.empty()) sub = bag;  // 目标稀有度已抽空：放宽为整个剩余池

        const Affix* picked = sub[static_cast<size_t>(
            GetRandomValue(0, static_cast<int>(sub.size()) - 1))];
        result.push_back(*picked);
        // 从剩余池移除同名项（判重按展示名，docs/03 4.6"互不重复"）
        bag.erase(std::remove_if(bag.begin(), bag.end(),
                                 [&](const Affix* a) { return a->name == picked->name; }),
                  bag.end());
    }

    // 记录本轮展示名，供下一轮过滤
    lastShown_.clear();
    for (const auto& a : result) lastShown_.push_back(a.name);
    return result;
}

int UpgradeSystem::applyChoice(const Affix& affix, GameContext& ctx) {
    if (ctx.player == nullptr) return 0;
    // T9 装备类：真实装备（无则创建 / 有则升级伤害），返回值供屏幕提示分支
    if (affix.kind == AffixKind::Weapon) {
        return ctx.player->giveWeapon(affix.weaponId);
    }
    if (affix.apply) {
        affix.apply(ctx.player->statsMutable());  // 属性联动：全部数值收敛于 Stats
    }
    if (affix.instantHpGain > 0.0f) {
        ctx.player->heal(affix.instantHpGain);    // 如"强健"：上限 +20 且立即回复 20
    }
    return 0;
}