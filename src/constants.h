#pragma once
// ============================================================
// constants.h —— 全局常量集中定义（分模块命名空间）
// 约定：所有数值基线集中于此，代码中禁止散落硬编码。
// T1 定义窗口与主循环常量；T2 收编玩家与竞技场数值基线（docs/03 4.1、4.8）。
// ============================================================
#include "raylib.h"  // Color 等 raylib 基础类型

namespace constants {

// ---- 窗口 ----
namespace window {
constexpr int    kLogicWidth  = 1280;  // 逻辑分辨率：宽
constexpr int    kLogicHeight = 720;   // 逻辑分辨率：高
constexpr int    kTargetFps   = 60;    // 期望显示帧率（仅影响渲染节奏，与逻辑步长解耦）
constexpr const char* kTitle = "竞技场幸存者 Arena Survivor";  // 窗口标题
}  // namespace window

// ---- 主循环 ----
namespace loop {
constexpr float kFixedDt      = 1.0f / 60.0f;  // 固定时间步长（秒），保证不同帧率逻辑一致
constexpr float kMaxFrameTime = 0.25f;         // 单帧最大累计时间（秒），超出截断防卡死
}  // namespace loop

// ---- 玩家（docs/03 4.1 数值基线）----
namespace player {
constexpr float kRadius    = 14.0f;   // 碰撞半径（px）
constexpr float kMoveSpeed = 220.0f;  // 基础移速（px/s），实际移速 = 基础移速 × Stats 移速倍率
constexpr float kStartX    = 1600.0f; // 出生点 X（场地中心）
constexpr float kStartY    = 900.0f;  // 出生点 Y（场地中心）
constexpr float kMaxHp     = 100.0f;  // 最大生命（docs/03 4.1）
constexpr float kInvincibleDuration = 0.6f;   // 受击无敌帧（秒）：> 敌人攻击冷却一半，防围殴连续掉血
constexpr float kKnockbackForce      = 240.0f; // 受击击退初速（px/s），总位移 ≈ 初速/衰减系数 = 24px
constexpr float kKnockbackDecay      = 10.0f;  // 击退速度指数衰减系数（/s），形成"被拍开"手感
}  // namespace player

// ---- 竞技场（docs/03 4.8 数值基线）----
namespace arena {
constexpr float kWidth         = 3200.0f;  // 场地宽（px）
constexpr float kHeight        = 1800.0f;  // 场地高（px）
constexpr float kWallThickness = 40.0f;    // 围墙厚度（px）
constexpr float kGridSpacing   = 100.0f;   // 地面网格线间距（px）
constexpr float kGridLineThickness = 1.0f; // 网格线粗细（px）

// ---- 配色：深色地面+暗网格区分场地；围墙醒目橙红；场外近黑 ----
constexpr Color kOutsideColor  {12, 14, 18, 255};   // 场外底色（近黑）
constexpr Color kGroundColor   {28, 32, 42, 255};   // 内场地面（深蓝灰）
constexpr Color kGridColor     {42, 48, 62, 255};   // 网格线（暗灰蓝）
constexpr Color kWallColor     {204, 68, 48, 255};  // 围墙主体（醒目橙红）
constexpr Color kWallEdgeColor {255, 140, 96, 255}; // 墙内缘高亮描边
}  // namespace arena

// ---- 敌人（docs/03 4.3 数值基线；T3 先落地爬行者）----
namespace enemy {
// 爬行者：近战追踪型
constexpr float kCrawlerHp     = 20.0f;   // 生命
constexpr float kCrawlerSpeed  = 120.0f;  // 移速（px/s），约为玩家 0.55 倍，可被甩开
constexpr float kCrawlerRadius = 14.0f;   // 碰撞半径（与玩家同规格）

// 分离力：同族距离小于该值时互相推离（约两半径和），避免敌人叠成一点
constexpr float kSeparationRadius   = 30.0f;
constexpr float kSeparationStrength = 0.6f;  // 分离力权重（相对追踪方向的力度，0~1）

// 接触战斗数值（docs/03 4.3；T5 启用）
constexpr float kCrawlerContactDamage  = 8.0f;  // 接触伤害（每击）
constexpr float kCrawlerAttackCooldown = 0.8f;  // 接触攻击冷却（秒），避免每帧掉血
constexpr int   kCrawlerXpValue        = 5;     // 击杀经验（T5 只作掉落展示，吸取 T7）

// 生成规则（T3 调试用，正式波次 T8 接入）
constexpr int   kDebugSpawnCount    = 10;     // F1 一次刷怪数量
constexpr float kSpawnMinPlayerDist = 400.0f; // 出生点距玩家最小距离，防骑脸
constexpr float kSpawnOutsideDepth  = 36.0f;  // 墙外出生：距墙外缘的深度（px），敌人从场外入场

// 配色：红色系与玩家蓝色明显区分；接触中描边变亮黄作为接触标记
constexpr Color kCrawlerColor        {220, 60, 50, 255};
constexpr Color kCrawlerOutlineColor {110, 30, 25, 255};
constexpr Color kContactColor        {255, 210, 90, 255};
}  // namespace enemy

// ---- 武器（docs/03 4.6 数值基线；T4 先落地手枪）----
namespace weapon {
constexpr float kPistolFireInterval = 1.0f / 2.4f;  // 攻击间隔（秒）≈0.4167，即射速 2.4 发/秒
constexpr float kPistolDamage       = 10.0f;        // 单发伤害（爬行者 20 HP 恰为两枪）
constexpr float kPistolRange        = 600.0f;       // 索敌射程（px）：只锁定射程内最近敌人
constexpr float kMuzzleOffset       = 20.0f;        // 枪口偏移：玩家圆心外移（半径 14 + 余量）
}  // namespace weapon

// ---- 子弹（docs/03 4.6：弹速 600、射程 900）----
namespace projectile {
constexpr float kBulletRadius   = 4.0f;    // 子弹碰撞半径
constexpr float kBulletSpeed    = 600.0f;  // 弹速（px/s），方向在开火时归一化
constexpr float kBulletMaxRange = 900.0f;  // 最大射程（px），超出自动消失
constexpr Color kBulletColor    {255, 210, 60, 255};  // 黄色小圆
}  // namespace projectile

// ---- 经验点（T5：击杀掉落视觉反馈；拾取与吸取 T7）----
namespace xp_orb {
constexpr float kSize = 12.0f;  // 绿色小方块边长（px）
constexpr Color kColor {60, 220, 120, 255};  // 绿色（与敌人红、子弹黄区分）
}  // namespace xp_orb

// ---- 升级（docs/03 4.5 经验曲线；T6 仅显示，升级逻辑 T7 迁 UpgradeSystem）----
namespace upgrade {
constexpr float kBaseExpNeeded = 20.0f;  // 升第 1 级所需经验
constexpr float kExpPerLevel   = 10.0f;  // 每级递增：升 L 级所需 = 20 + 10×(L-1)
}  // namespace upgrade

// ---- HUD（T6：屏幕空间 UI；颜色/字号保证深色背景下清晰）----
namespace ui {
constexpr const char* kFontPathYaHei  = "C:/Windows/Fonts/msyh.ttc";   // 中文字体：微软雅黑
constexpr const char* kFontPathSimHei = "C:/Windows/Fonts/simhei.ttf"; // 回退字体：黑体
// 中文字符集（供字体图集加载；后续任务新增 UI 文案时需在此追加对应汉字）
constexpr const char* kCnCharSet =
    "第波剩余时间敌人击杀手枪自动射击死亡逻辑将在后续任务实现方向键移动刷怪退出·（）";
constexpr int   kFontBaseSize  = 32;   // 字体图集基准字号（px），绘制时按需缩放
constexpr int   kHudMargin     = 16;   // HUD 距屏幕边缘边距（px）
constexpr int   kExpBarWidth   = 260;  // 经验条宽（px）
constexpr int   kExpBarHeight  = 12;   // 经验条高（px）
constexpr int   kHpBarWidth    = 260;  // 血条宽（px）
constexpr int   kHpBarHeight   = 18;   // 血条高（px）
constexpr float kHpLerpSpeed   = 8.0f; // 血条平滑趋近速度（/s，指数趋近避免跳变）
constexpr Color kExpBarColor   {240, 180, 40, 255};  // 经验条填充（黄）
constexpr Color kBarBgColor    {40, 44, 54, 255};    // 条底（深灰）
constexpr Color kHpBarBgColor  {150, 32, 32, 255};   // 血条底（暗红）
constexpr Color kHpFillColor   {60, 190, 80, 255};   // 血条填充（绿）
constexpr Color kTextMainColor {230, 230, 230, 255}; // 主文字（近白）
constexpr Color kTextDimColor  {170, 170, 170, 255}; // 次级文字（灰）
}  // namespace ui

}  // namespace constants
