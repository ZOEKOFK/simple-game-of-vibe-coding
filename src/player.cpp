// player.cpp —— Player 实现（T2：移动+自绘；T4：初始手枪与武器驱动；T5：受击/无敌帧/击退；T9 装备发放）
#include "player.h"

#include <cmath>
#include <memory>
#include <string>

#include "arena.h"
#include "assets.h"   // T12：玩家贴图（替代几何圆形绘制）
#include "constants.h"
#include "raymath.h"  // Vector2Length（raylib.h 未包含 raymath，需显式引入）
#include "weapon_pistol.h"  // 初始武器：手枪

Player::Player() {
    // 开局初始装备：手枪（docs/01 FR-05；本局武器列表随升级扩充）
    weapons_.push_back(std::make_unique<Pistol>(&stats_));
}

void Player::reset() {
    // T10：重开局全量复位——位置/HP/朝向/击退/无敌帧/Stats/武器列表回到构造态
    //（equip 词条/吸血/穿透/飞刃强化等全部随 Stats 重建清空，武器还原初始手枪）
    pos_ = Vector2{constants::player::kStartX, constants::player::kStartY};
    lastMoveDir_ = Vector2{1.0f, 0.0f};
    hp_ = constants::player::kMaxHp;
    stats_ = Stats{};
    knockbackVel_ = Vector2{};
    invincibleTimer_ = 0.0f;
    weapons_.clear();
    weapons_.push_back(std::make_unique<Pistol>(&stats_));
}

void Player::updateWeapons(float dt, GameContext& ctx) {
    // 逐武器驱动：索敌/冷却/开火逻辑封装在 Weapon 基类主流程中
    for (auto& w : weapons_) {
        w->update(dt, ctx, *this);
    }
}

const Vector2& Player::pos() const { return pos_; }  // 供镜头跟随等只读查询
float Player::radius() const { return constants::player::kRadius; }
int   Player::maxHp() const {  // 最大生命 = 基础值 + Stats 生命加成（加成词条 T7）
    return static_cast<int>(constants::player::kMaxHp + stats_.extraMaxHp);
}
int   Player::hp() const { return static_cast<int>(hp_); }  // 取整显示（伤害均为整值）
float Player::pickupRadius() const {  // T7：拾取半径 = 基础 70 + Stats 加成（磁力词条/装备可扩）
    return constants::player::kPickupRadius + stats_.pickupRadius;
}
const Stats& Player::stats() const { return stats_; }

void Player::heal(float amount) {
    hp_ += amount;
    const float cap = static_cast<float>(maxHp());  // 上限截断："强健"提高上限后可回得更多
    if (hp_ > cap) hp_ = cap;
}

int Player::giveWeapon(const std::string& weaponId) {
    // 已有该武器 → 升级伤害（Weapon::upgradePick；飞刃/霰弹同一成长规则）
    for (auto& w : weapons_) {
        if (w && w->typeId() == weaponId) {
            w->upgradePick();
            return 2;
        }
    }
    // 未持有 → 经工厂注册表创建（新增武器不改本函数；无效 id 返回 nullptr）
    auto w = Weapon::create(weaponId, &stats_);
    if (!w) return 0;
    weapons_.push_back(std::move(w));
    return 1;
}

void Player::drawWeapons() const {
    // T9：持续型武器自绘（发射型 draw 为空实现，无开销）
    for (const auto& w : weapons_) {
        w->draw(*this);
    }
}

void Player::takeDamage(float amount, Vector2 fromPos) {
    if (invincibleTimer_ > 0.0f) return;  // 无敌帧内免伤（docs/03 4.1：0.6s）

    // 实际伤害 = 原始伤害 × 受伤倍率（Stats 默认 1.0；减伤词条 T7 生效）
    hp_ -= amount * stats_.damageTakenMult;
    if (hp_ < 0.0f) hp_ = 0.0f;  // HP 锁 0：死亡逻辑留待 T10，本任务仅锁定为 0

    invincibleTimer_ = constants::player::kInvincibleDuration;  // 进入无敌帧

    // 击退：沿"攻击者 → 玩家"方向施加初速（fromPos 为攻击来源），随后逐帧衰减
    Vector2 dir{pos_.x - fromPos.x, pos_.y - fromPos.y};
    const float len = Vector2Length(dir);
    if (len > 0.0001f) {
        dir.x /= len;
        dir.y /= len;
    } else {
        dir = Vector2{1.0f, 0.0f};  // 与攻击来源重合时兜底朝右
    }
    knockbackVel_ = Vector2{dir.x * constants::player::kKnockbackForce,
                            dir.y * constants::player::kKnockbackForce};
}

void Player::update(float dt, Vector2 inputDir) {
    // 对角归一化：方向向量先归一化再乘移速，保证斜向不加速
    const float len = Vector2Length(inputDir);
    if (len > 0.0001f) {
        inputDir.x /= len;
        inputDir.y /= len;
        lastMoveDir_ = inputDir;  // 静止时保持最后一次朝向
    }

    // 实际移速 = 基础移速 × Stats 移速倍率（默认倍率 1.0，词条 T7 接入）
    const float speed = constants::player::kMoveSpeed * stats_.moveSpeedMult;
    pos_.x += inputDir.x * speed * dt;
    pos_.y += inputDir.y * speed * dt;

    // 边界钳制：含碰撞半径，玩家不可嵌入围墙
    pos_ = clampToArena(pos_, constants::player::kRadius);

    // 无敌帧计时递减（T5：受击后 0.6s 内免伤；draw 中闪烁提示）
    if (invincibleTimer_ > 0.0f) invincibleTimer_ -= dt;

    // 击退衰减（T5）：受击瞬间获得初速，此后指数衰减并叠加到位移上，
    // 形成"被拍开"手感；总位移 ≈ 初速 / 衰减系数 = 240/10 = 24px
    const float decay = std::exp(-constants::player::kKnockbackDecay * dt);
    knockbackVel_.x *= decay;
    knockbackVel_.y *= decay;
    pos_.x += knockbackVel_.x * dt;
    pos_.y += knockbackVel_.y * dt;

    // 击退位移后再次钳制，防止被撞出墙外
    pos_ = clampToArena(pos_, constants::player::kRadius);
}

void Player::draw() const {
    // 无敌帧闪烁（T5）：每 0.2s 隐藏半周期，视觉提示"无敌中"
    if (invincibleTimer_ > 0.0f && std::fmod(invincibleTimer_, 0.2f) < 0.1f) return;

    // T12：玩家贴图替代几何圆形——以朝向旋转、尺寸≈2.2×碰撞半径（视觉略大于碰撞盒）
    const float r = constants::player::kRadius;
    const float deg = std::atan2(lastMoveDir_.y, lastMoveDir_.x) * (180.0f / PI) + 90.0f;
    assets::drawCentered(assets::player(), pos_, r * 2.2f, deg, WHITE);
}