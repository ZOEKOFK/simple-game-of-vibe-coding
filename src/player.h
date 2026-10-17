#pragma once
// ============================================================
// player.h —— 玩家
// 职责：八方向移动、受击/无敌帧/击退，持有武器与 Stats。
// T2 已实现移动与自绘；T4 持有初始手枪并逐帧驱动武器；受击 T5、装备词条 T7。
// ============================================================
#include <memory>
#include <vector>

#include "constants.h"  // 数值基线：半径/移速/出生点
#include "raylib.h"     // Vector2
#include "weapon.h"     // unique_ptr<Weapon> 析构需要完整类型
#include "stats.h"

struct GameContext;  // 前向声明（定义在 game.h），武器驱动经此读写世界

class Player {
public:
    Player();                                       // 开局自带初始武器（手枪）

    // ---- 行为接口 ----
    void update(float dt, Vector2 inputDir);       // T2：移动 + 无敌帧/击退衰减
    void updateWeapons(float dt, GameContext& ctx);  // T4：逐武器索敌/冷却/开火
    void draw() const;                             // T2：自绘（含无敌闪烁）

    void takeDamage(float amount, Vector2 fromPos);  // T5：无敌帧内忽略；否则扣血并击退
    void addWeapon(std::unique_ptr<Weapon> w);       // T7：装备类词条由此进入

    // ---- 查询接口（只读，供武器/UI/碰撞系统使用）----
    const Vector2& pos() const;   // 世界坐标
    float radius() const;         // 碰撞半径
    int   maxHp() const;          // 最大生命 = 基础值 + Stats 加成
    int   hp() const;             // 当前生命
    float pickupRadius() const;   // 拾取半径
    const Stats& stats() const;   // 属性中心（词条修改点）

private:
    Vector2 pos_{constants::player::kStartX,
                 constants::player::kStartY};  // 世界坐标（T2 出生点 = 场地中心）
    Vector2 lastMoveDir_{1.0f, 0.0f};          // 朝向 = 最后非零移动方向（朝向指示线用）
    float   hp_ = 100.0f;           // 当前生命（基线见 docs/03 数值表，T2 收编入 constants.h）
    Stats   stats_{};               // 属性中心
    Vector2 knockbackVel_{};        // 击退速度（受击瞬间赋值，逐帧衰减）
    float   invincibleTimer_ = 0.0f;  // 无敌帧计时（>0 时不受伤）
    std::vector<std::unique_ptr<Weapon>> weapons_{};  // 本局武器列表
};
