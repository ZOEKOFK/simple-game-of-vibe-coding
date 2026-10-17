#pragma once
// ============================================================
// affix_library.h —— 词条结构与词条池（数据模块）
// 设计要点：所有词条统一实现为"修改 Stats 的函数"；
//           装备类词条表现为添加/升级武器。
// T1 仅声明结构与稀有度枚举；词条池定义表在 T7 实现。
// ============================================================
#include <functional>
#include <string>

#include "stats.h"  // 词条 apply 的目标类型

// 稀有度枚举（抽取权重 普通 60% / 稀有 30% / 史诗 10%，见 docs/03 数值表）
enum class Rarity { Common, Rare, Epic };

struct Affix {
    std::string name;                    // 展示名，如 "伤害 +15%"
    Rarity rarity = Rarity::Common;      // 稀有度
    std::function<void(Stats&)> apply;   // 属性类词条生效入口（T7 填充）
    // 装备类词条（如"霰弹枪"/"环绕飞刃"）通过额外标志在 UpgradeSystem 中特殊处理（T7 细化）
};
