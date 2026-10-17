#pragma once
// ============================================================
// weapon.h —— Weapon 抽象基类（武器系统的核心扩展点）
// 设计要点：自动攻击决策（索敌）封装在基类；子类只实现"如何开火"。
// 新增武器只需：新增子类文件 → 覆写 fire()/update() → 注册名称，不改旧文件。
// T1 仅声明接口；索敌/开火在 T4 实现。
// 注意：本头文件对 GameContext 只做前向声明，避免分层依赖倒置。
// ============================================================
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "raylib.h"  // Vector2

struct GameContext;  // 前向声明（定义在 game.h）：武器只读查询敌人/玩家并发起子弹生成
struct Stats;        // 前向声明：以指针持有即可
class Player;

class Weapon {
public:
    explicit Weapon(float fireInterval, float damage, float range);
    virtual ~Weapon() = default;

    // 非虚主流程：索敌(射程内最近敌人) → 冷却计时 → 到点开火；持续型武器可整体覆写（T4）
    virtual void update(float dt, GameContext& ctx, const Player& player);

    // ---- 类型信息（供 UI 展示与强化判重）----
    const std::string& typeId() const;  // 注册名，如 "pistol" / "shotgun" / "orbital"

    // ---- 展示信息（T6 HUD 显示；默认回退注册名，子类按需覆写）----
    virtual std::string displayName() const { return typeId_; }  // 显示名（如"手枪"）
    virtual std::string description() const { return ""; }       // 一句话说明（如"自动射击"）

    // ---- T9 扩展接口（默认实现均无害，新增武器零成本接入）----
    virtual void draw(const Player& player) const;   // 持续型武器自绘（环绕飞刃；默认不画）
    virtual void upgradePick();                      // 装备词条再选：伤害 += upgradeDamageAmount()
    virtual float upgradeDamageAmount() const;       // 再选升级增量（按武器差异化：手枪 6/霰弹 3/飞刃 4）

    // ---- 工厂注册机制（新增武器只增不改的关键）----
    using Factory = std::function<std::unique_ptr<Weapon>(const Stats* stats)>;
    static std::unordered_map<std::string, Factory>& registry();   // 注册表（定义于 weapon.cpp）
    static void registerWeapon(const std::string& id, Factory f);  // 静态注册入口
    static std::unique_ptr<Weapon> create(const std::string& id, const Stats* stats);

protected:
    // 发射型子类（手枪/霰弹）实现开火模式；环绕飞刃整体覆写 update()
    virtual void fire(GameContext& ctx, const Player& player, const Vector2& targetPos);

    const Stats* stats_ = nullptr;  // 只读：实际伤害 = 基础伤害 × damageMult
    float fireTimer_    = 0.0f;     // 下次可开火时间倒计时
    float fireInterval_ = 0.5f;     // 基础攻击间隔（秒）
    float baseDamage_   = 10.0f;    // 基础伤害
    float range_        = 600.0f;   // 索敌射程（像素）
    std::string typeId_;            // 注册名
};
