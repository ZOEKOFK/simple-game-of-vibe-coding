#pragma once
// ============================================================
// upgrade_system.h —— 升级与随机词条系统
// 职责：经验→等级→三选一抽取→词条应用（T7 实现）。
// 升级请求用 pendingUpgrades_ 计数：经验可能一次攒满多级，
//   GameEngine 每结束一轮三选一就 consumePendingUpgrade 一次，
//   仍有剩余则留在 UPGRADE 重抽，实现"连升多级逐一三选一"。
// ============================================================
#include <vector>

#include "affix_library.h"

struct GameContext;

class UpgradeSystem {
public:
    void addExp(int amount, GameContext& ctx);  // 经验入账；溢出保留，攒满 1 级登记 1 次升级请求

    int level() const;                          // 当前等级
    int expInLevel() const;                     // 当前等级内经验
    int expToNext() const { return 20 + 10 * (level_ - 1); }  // 升 L 级所需经验（docs/03 4.5）

    // 升级请求：GameEngine 据此切入 UPGRADE（可能一次攒满多级）
    bool hasPendingUpgrades() const { return pendingUpgrades_ > 0; }
    bool consumePendingUpgrade();               // 消耗一次请求；返回"是否仍有剩余"

    // 三选一抽取（两段式：先按权重抽稀有度 60/30/10，再子池内等概率抽词条）：
    //   - 三选项互不重复（按展示名判重）；
    //   - requiresWeapon 不满足的词条不入池（docs/03 4.7 #11 前置条件模式）；
    //   - 剔除上一轮已展示词条防连续重复，候选不足时放宽允许重复。
    std::vector<Affix> drawChoices(int count, GameContext& ctx);

    // 应用选项：属性类 → affix.apply(Stats)；instantHpGain > 0 → 立即回血；
    //   装备类（kind=Weapon）→ Player::giveWeapon（T9 真实装备/升级伤害）。
    // 返回 1=新获得武器 / 2=已有武器升级 / 0=其余（供 GameEngine 提示文案分支）
    int applyChoice(const Affix& affix, GameContext& ctx);

private:
    int level_ = 1;
    int exp_ = 0;
    int pendingUpgrades_ = 0;            // 待处理升级次数（连升多级缓冲）
    std::vector<std::string> lastShown_; // 上一轮三选一展示名（防连续重复）
};