#pragma once
// ============================================================
// xp_orb.h —— 经验点
// T5：击杀敌人时掉落的经验点，暂仅作视觉反馈（绿色小方块）；
//     拾取/吸取判定与升级结算 T7 接入。
// ============================================================
#include "raylib.h"

class XPOrb {
public:
    XPOrb(const Vector2& pos, float value);

    void draw() const;               // T13 程序化六边形青绿宝石（脉动+自转+光晕）

    // ---- 查询接口 ----
    const Vector2& pos() const { return pos_; }  // T7 拾取判定用
    float value() const { return value_; }        // 掉落经验值

private:
    Vector2 pos_{};      // 世界坐标（敌人死亡位置）
    float   value_ = 0.0f;  // 经验值（爬行者 5，见 docs/03 4.3）
};