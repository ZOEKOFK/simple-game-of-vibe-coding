#pragma once
// ============================================================
// stats.h —— Stats 属性系统
// 全部可成长属性的唯一权威来源，词条统一的修改落点。
// T1 仅声明结构体；词条应用逻辑在 T7 实现。
// ============================================================

struct Stats {
    // ---- 倍率类（与基础值相乘）----
    float damageMult      = 1.0f;  // 伤害倍率
    float attackSpeedMult = 1.0f;  // 攻速倍率（实际间隔 = 基础间隔 / 倍率）
    float moveSpeedMult   = 1.0f;  // 移速倍率（实际移速 = 基础移速 × 倍率）
    float bulletSpeedMult = 1.0f;  // 弹速倍率

    // ---- 加算类（与基础值相加）----
    float extraMaxHp   = 0.0f;  // 最大生命加成
    float pickupRadius = 0.0f;  // 拾取半径加成（实际 = 基础半径 + 加成）
    float hpRegen      = 0.0f;  // 每秒生命回复

    // ---- 特殊类 ----
    float damageTakenMult = 1.0f;  // 受伤倍率（<1 即减伤）

    // ---- T9 新增：武器机制类词条落点 ----
    int   penetrationExtra = 0;      // 子弹穿透数（穿甲弹 +1：单发可穿过 1 个敌人）
    float lifestealPct     = 0.0f;   // 吸血比例（吸血 2%：造成伤害的 2% 回复生命）
    float bladeDamageBonus = 0.0f;   // 环绕飞刃加算伤害（飞刃强化 +5）
};
