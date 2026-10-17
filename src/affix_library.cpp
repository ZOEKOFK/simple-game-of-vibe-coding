// affix_library.cpp —— 词条池 v2 定义表（docs/03 4.7：T9 扩充至 17 项）
#include "affix_library.h"

#include "constants.h"  // 词条数值基线全部收编于此（禁止散落硬编码）

namespace {

using namespace constants::affix;

// 构建词条池：每项 = 展示文案 + 稀有度 + 类别 + Stats 修改函数/武器注册名
// 构成（docs/03 4.7 v2）：
//   属性类 12 = 伤害强化/急速/疾跑/强健/高速子弹（普通 5）
//             + 磁力/生命再生/韧性（稀有 3）
//             + 神速/铁甲/巨人/强磁力（史诗 4）
//   特殊类 3  = 吸血/穿甲弹（稀有-史诗）/飞刃强化（稀有，前置环绕飞刃）
//   装备类 2  = 霰弹枪/环绕飞刃（史诗，kind=Weapon，选到真实装备/升级）
std::vector<Affix> buildPool() {
    using R = Rarity;
    using K = AffixKind;
    std::vector<Affix> pool;

    // ---- 普通（60% 权重子池，稳定小幅增益）----
    pool.push_back(
        {"伤害强化", "Sharp Strikes", "伤害 +15%", "Damage +15%",
         R::Common, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.damageMult *= kDamageBoost; }});
    pool.push_back(
        {"急速", "Haste", "攻速 +10%", "Attack speed +10%",
         R::Common, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.attackSpeedMult *= kAttackSpeedBoost; }});
    pool.push_back(
        {"疾跑", "Sprint", "移速 +8%", "Move speed +8%",
         R::Common, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.moveSpeedMult *= kMoveSpeedBoost; }});
    pool.push_back(
        {"强健", "Vigor", "生命上限 +20，并回复 20", "Max HP +20, heal 20",
         R::Common, K::Stat, "", "", kHpBonus,
         [](Stats& s) { s.extraMaxHp += kHpBonus; }});  // 立即回血由 instantHpGain 结算
    pool.push_back(
        {"高速子弹", "Swift Bullets", "弹速 +25%", "Bullet speed +25%",
         R::Common, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.bulletSpeedMult *= kBulletSpeedBoost; }});

    // ---- 稀有（30% 权重子池，明显变强）----
    pool.push_back(
        {"磁力", "Magnet", "拾取半径 +30", "Pickup radius +30",
         R::Rare, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.pickupRadius += kPickupBonus; }});
    pool.push_back(
        {"生命再生", "Regeneration", "每秒回复 1 HP", "Regen +1 HP/s",
         R::Rare, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.hpRegen += kHpRegenBonus; }});
    pool.push_back(
        {"韧性", "Toughness", "受伤 -10%", "Damage taken -10%",
         R::Rare, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.damageTakenMult *= kDamageTakenReduce; }});
    // ---- 特殊类：生命偷取（T9；飞刃/子弹统一按结算伤害比例回复）----
    pool.push_back(
        {"吸血", "Lifesteal", "伤害 2% 回复生命", "2% lifesteal",
         R::Rare, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.lifestealPct += kLifestealPct; }});
    // ---- 特殊类：前置条件词条（需已有环绕飞刃才入池，docs/03 4.7 #11）----
    pool.push_back(
        {"飞刃强化", "Blade Power", "单刃伤害 +5", "Blade damage +5",
         R::Rare, K::Stat, "orbital", "", 0.0f,
         [](Stats& s) { s.bladeDamageBonus += kBladeDamageBonus; }});

    // ---- 史诗（10% 权重子池，质变头部词条）----
    pool.push_back(
        {"神速", "Swiftness", "攻速 +25%", "Attack speed +25%",
         R::Epic, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.attackSpeedMult *= kAttackSpeedBig; }});
    pool.push_back(
        {"铁甲", "Iron Skin", "受伤 -20%", "Damage taken -20%",
         R::Epic, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.damageTakenMult *= kDamageTakenBig; }});
    pool.push_back(
        {"巨人", "Giant", "生命上限 +40，并回复 20", "Max HP +40, heal 20",
         R::Epic, K::Stat, "", "", kHpBonus,  // 回复 20（不是 40）：血条不满时的即战力
         [](Stats& s) { s.extraMaxHp += kHpBonusBig; }});
    pool.push_back(
        {"强磁力", "Super Magnet", "拾取半径 +60", "Pickup radius +60",
         R::Epic, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.pickupRadius += kPickupBonusBig; }});
    // ---- 特殊类：穿透（T9；对每颗弹丸独立生效）----
    pool.push_back(
        {"穿甲弹", "Piercing Shot", "子弹穿透 +1", "Bullets pierce +1",
         R::Epic, K::Stat, "", "", 0.0f,
         [](Stats& s) { s.penetrationExtra += kPenetrationBonus; }});

    // ---- 装备类（kind=Weapon：选到即获得武器，已有则升级伤害；T9 实装）----
    pool.push_back(
        {"霰弹枪", "Shotgun", "获得霰弹枪：5 发扇形散射",
         "Get Shotgun: 5-pellet spread",
         R::Epic, K::Weapon, "", "shotgun", 0.0f, nullptr});
    pool.push_back(
        {"环绕飞刃", "Orbital Blades", "获得环绕飞刃",
         "Get Orbital Blades",
         R::Epic, K::Weapon, "", "orbital", 0.0f, nullptr});

    return pool;
}

}  // namespace

const std::vector<Affix>& affixPool() {
    // 静态单例池：首次访问构建一次，此后只读
    static const std::vector<Affix> pool = buildPool();
    return pool;
}