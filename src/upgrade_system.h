#pragma once
// ============================================================
// upgrade_system.h —— 升级与随机词条系统
// 职责：经验→等级→三选一抽取→词条应用（T7/T9 实现）。
// ============================================================
#include <vector>

#include "affix_library.h"

struct GameContext;

class UpgradeSystem {
public:
    void addExp(int amount, GameContext& ctx);  // 含"经验攒满触发升级请求"判定（T7）

    int level() const;                          // 当前等级
    int expInLevel() const;                     // 当前等级内经验
    int expToNext() const;                      // 升 L 级所需经验 = 20 + 10×(L-1)

    std::vector<Affix> drawChoices(int count = 3);          // 三选一：稀有度权重 + 互不重复（T7）
    void applyChoice(const Affix& affix, GameContext& ctx); // 属性类→apply；装备类→加武器（T7）

private:
    int level_ = 1;
    int exp_ = 0;
};
