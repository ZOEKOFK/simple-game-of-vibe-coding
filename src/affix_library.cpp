// affix_library.cpp —— 词条池 v1 定义表（docs/03 4.7 全 11 项；T9 扩充至 16+）
#include "affix_library.h"

#include "constants.h"  // 词条数值基线全部收编于此（禁止散落硬编码）

namespace {

using namespace constants::affix;

// 构建词条池：每项 = 展示文案 + 稀有度 + Stats 修改函数
std::vector<Affix> buildPool() {
    using R = Rarity;
    using K = AffixKind;
    std::vector<Affix> pool;

    // ---- 普通（60% 权重子池，稳定小幅增益）----
    pool.push_back(
        {"伤害强化", "Sharp Strikes", "伤害 +15%", "Damage +15%",
         R::Common, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.damageMult *= kDamageBoost; }});
    pool.push_back(
        {"急速", "Haste", "攻速 +10%", "Attack speed +10%",
         R::Common, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.attackSpeedMult *= kAttackSpeedBoost; }});
    pool.push_back(
        {"疾跑", "Sprint", "移速 +8%", "Move speed +8%",
         R::Common, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.moveSpeedMult *= kMoveSpeedBoost; }});
    pool.push_back(
        {"强健", "Vigor", "生命上限 +20，并回复 20", "Max HP +20, heal 20",
         R::Common, K::Stat, "", "", "", kHpBonus,
         [](Stats& s) { s.extraMaxHp += kHpBonus; }});  // 立即回血由 instantHpGain 结算
    pool.push_back(
        {"高速子弹", "Swift Bullets", "弹速 +25%", "Bullet speed +25%",
         R::Common, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.bulletSpeedMult *= kBulletSpeedBoost; }});

    // ---- 稀有（30% 权重子池，明显变强）----
    pool.push_back(
        {"磁力", "Magnet", "拾取半径 +30", "Pickup radius +30",
         R::Rare, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.pickupRadius += kPickupBonus; }});
    pool.push_back(
        {"生命再生", "Regeneration", "每秒回复 1 HP", "Regen +1 HP/s",
         R::Rare, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.hpRegen += kHpRegenBonus; }});
    pool.push_back(
        {"韧性", "Toughness", "受伤 -10%", "Damage taken -10%",
         R::Rare, K::Stat, "", "", "", 0.0f,
         [](Stats& s) { s.damageTakenMult *= kDamageTakenReduce; }});

    // ---- 史诗（10% 权重子池，装备类；T7 占位补偿，T9 换真实武器）----
    pool.push_back(
        {"霰弹枪", "Shotgun", "获得霰弹枪（暂补偿伤害 15%）",
         "Get Shotgun (comp. +15% dmg)",
         R::Epic, K::Weapon, "", "已获取「霰弹枪」：武器将在 T9 实装，本次已补偿伤害 +15%",
         "Shotgun: lands in T9, +15% damage granted for now", 0.0f,
         [](Stats& s) { s.damageMult *= kWeaponPlaceholderDamage; }});
    pool.push_back(
        {"环绕飞刃", "Orbital Blades", "获得环绕飞刃（暂补偿攻速 10%）",
         "Get Orbital Blades (comp. +10% atk spd)",
         R::Epic, K::Weapon, "", "已获取「环绕飞刃」：武器将在 T9 实装，本次已补偿攻速 +10%",
         "Orbital Blades: lands in T9, +10% attack speed granted for now", 0.0f,
         [](Stats& s) { s.attackSpeedMult *= kWeaponPlaceholderAttackSpeed; }});

    // ---- 前置条件词条雏形（docs/03 4.7 #11）：需已有环绕飞刃才入池 ----
    // T7 无飞刃武器，该词条不可达；T9 飞刃实装后自然解禁
    pool.push_back(
        {"飞刃强化", "Blade Power", "单刃伤害 +5", "Blade damage +5",
         R::Rare, K::Stat, "orbital", "", "", 0.0f,
         [](Stats& s) { s.damageMult *= 1.0f; /* T9：改为对环绕飞刃本体的强化 */ (void)s; }});

    return pool;
}

}  // namespace

const std::vector<Affix>& affixPool() {
    // 静态单例池：首次访问构建一次，此后只读
    static const std::vector<Affix> pool = buildPool();
    return pool;
}