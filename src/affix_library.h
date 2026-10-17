#pragma once
// ============================================================
// affix_library.h —— 词条结构与词条池（数据模块）
// 设计要点（docs/03）：所有词条统一实现为"修改 Stats 的函数"，
//   全部数值联动收敛于 Stats 单一聚合点，便于平衡审查；
//   装备类词条（kind=Weapon + weaponId）在 T9 实装：选到即获得武器/升级伤害。
// ============================================================
#include <functional>
#include <string>
#include <vector>

#include "stats.h"  // 词条 apply 的目标类型

// 稀有度：抽取权重 普通 60% / 稀有 30% / 史诗 10%（docs/03 4.6）
enum class Rarity { Common, Rare, Epic };

// 词条类别：属性类（修改 Stats）/ 装备类（获得武器）
enum class AffixKind { Stat, Weapon };

struct Affix {
    std::string name;           // 中文展示名
    std::string nameEn;         // 英文回退名（中文字体加载失败时使用）
    std::string description;    // 卡片描述（中文，≤18 字保证单行放下）
    std::string descriptionEn;  // 卡片描述（英文回退）
    Rarity rarity = Rarity::Common;
    AffixKind kind = AffixKind::Stat;
    std::string requiresWeapon; // 前置武器 typeId（空=无条件；不满足则不入抽取池，docs/03 4.7 #11 模式）
    std::string weaponId;       // 装备类：对应武器 typeId（选到即 Player::giveWeapon）
    float instantHpGain = 0.0f; // 选择时立即回血（如"强健"：上限 +20 且回复 20）
    std::function<void(Stats&)> apply;  // 属性修改入口（装备类为空）
};

// 词条池 v2（17 项：属性 12 + 装备 2 + 特殊 3；docs/03 4.7），定义于 affix_library.cpp
const std::vector<Affix>& affixPool();