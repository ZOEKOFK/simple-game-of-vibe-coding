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
constexpr float kPickupRadius = 70.0f; // 基础拾取半径（px），实际 = 基础 + Stats 加成（T7 吸取）
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
constexpr Color kGroundColor   {22, 26, 36, 255};   // 内场地面（T13 调深：衬托像素贴图与弹体发光）
constexpr Color kGridColor     {46, 52, 70, 150};   // 网格线（暗灰蓝半透明：更淡不抢眼）
constexpr Color kWallColor     {204, 68, 48, 255};  // 围墙主体（醒目橙红）
constexpr Color kWallEdgeColor {255, 140, 96, 255}; // 墙内缘高亮描边
constexpr Color kWallOuter     {128, 36, 28, 255};  // T13 墙外缘深色层（厚度感）
constexpr Color kWallSeam      {116, 34, 26, 150};  // T13 墙身砖缝线（半透明）
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
constexpr int   kStressSpawnCount   = 200;    // F2 性能压力刷怪数量（T11：同屏 200 敌人验证）
constexpr float kSpawnMinPlayerDist = 400.0f; // 出生点距玩家最小距离，防骑脸
constexpr float kSpawnOutsideDepth  = 36.0f;  // 墙外出生：距墙外缘的深度（px），敌人从场外入场

// 配色：红色系与玩家蓝色明显区分；接触中描边变亮黄作为接触标记
constexpr Color kCrawlerColor        {220, 60, 50, 255};
constexpr Color kCrawlerOutlineColor {110, 30, 25, 255};
constexpr Color kContactColor        {255, 210, 90, 255};

// ---- 喷吐者：远程风筝射击怪（docs/03 4.3；T8，第 5 波起混入）----
constexpr float kSpitterHp          = 30.0f;   // 生命
constexpr float kSpitterSpeed       = 100.0f;  // 移速（px/s）
constexpr float kSpitterRadius      = 14.0f;   // 碰撞半径
constexpr float kSpitterMinDist     = 300.0f;  // 风筝下界：小于则远离玩家
constexpr float kSpitterMaxDist     = 600.0f;  // 风筝上界：大于则接近玩家
constexpr float kSpitterShootInterval      = 2.0f;   // 射击间隔（秒）
constexpr float kSpitterProjectileSpeed   = 180.0f; // 敌弹弹速（≈玩家移速 0.8 倍：不躲会中、跑了躲得开）
constexpr float kSpitterProjectileDamage  = 6.0f;   // 敌弹伤害
constexpr float kSpitterProjectileRange   = 900.0f; // 敌弹最大射程（px）
constexpr int   kSpitterXpValue     = 7;      // 击杀经验

// ---- 蛮牛：精英冲锋怪（docs/03 4.3；T8，第 8 波起混入）----
constexpr float kBruteHp           = 600.0f;  // 生命 = 爬行者 ×30
constexpr float kBruteSpeed        = 84.0f;   // 移速 = 爬行者 ×0.7
constexpr float kBruteRadius       = 28.0f;   // 碰撞半径 = 爬行者 ×2
constexpr float kBruteContactDamage = 24.0f;  // 接触伤害 = 爬行者 ×3
constexpr float kBruteWindupTime    = 0.8f;   // 蓄力时长（秒）：原地钉住、变色预警
constexpr float kBruteChargeTime    = 1.0f;   // 冲锋时长（秒）
constexpr float kBruteChargeSpeedMult = 3.0f; // 冲锋速度倍率：84×3=252 > 玩家 220，必须预读闪避
constexpr float kBruteChargeCooldown  = 5.0f; // 冲锋冷却（秒）
constexpr float kBruteWindupTriggerDist = 320.0f; // 蓄力触发距离：进入该距离开始蓄力（留预读窗口）
constexpr int   kBruteXpValue      = 50;      // 击杀经验 = 爬行者 ×10
// 配色：紫色识别远程兵；蛮牛深红，蓄力/冲锋变亮红是"可读的威胁"
constexpr Color kSpitterColor        {170, 90, 200, 255};
constexpr Color kSpitterOutlineColor {85, 45, 100, 255};
constexpr Color kBruteColor          {160, 40, 40, 255};
constexpr Color kBruteOutlineColor   {80, 20, 20, 255};
constexpr Color kBruteWindupColor    {255, 80, 60, 255};
constexpr Color kBruteChargeColor    {255, 30, 20, 255};
constexpr Color kBruteWindupRingColor{255, 90, 70, 160};
}  // namespace enemy

// ---- 波次（docs/03 4.4：数量/生命曲线与构成权重；T8 WaveManager 生效）----
namespace wave {
constexpr float kRestTime         = 3.0f;   // 波间休整倒计时（秒）
constexpr float kBatchInterval    = 0.5f;   // 刷怪批次间隔（秒）：分批投放避免一帧全刷
constexpr int   kBatchSize        = 4;      // 每批次刷怪数（不足一批时一次放完）
constexpr int   kBaseCount        = 8;      // 第 1 波敌人数
constexpr int   kCountPerWave     = 4;      // 每波递增数：总数 = 8 + 4×(n-1)
constexpr float kHpMultBase       = 1.0f;   // 生命倍率基数
constexpr float kHpMultPerWave    = 0.25f;  // 每波生命倍率递增：1 + 0.25×(n-1)
// 构成权重（docs/03 4.4；同波次内逐只随机，体现 Roguelite 随机性）
constexpr int   kSpitterFirstWave = 5;      // 喷吐者首次混入波次
constexpr int   kBruteFirstWave    = 8;      // 蛮牛首次混入波次
constexpr int   kBruteMaxPerWave   = 2;      // 蛮牛每波投放上限
constexpr float kCrawlerWeightMid  = 0.80f;  // 5~7 波爬行者权重
constexpr float kCrawlerWeightLate = 0.70f;  // 8+ 波爬行者权重
constexpr float kSpitterWeightMid  = 0.20f;  // 5~7 波喷吐者权重
constexpr float kSpitterWeightLate = 0.22f;  // 8+ 波喷吐者权重
constexpr float kBruteWeightLate   = 0.08f;  // 8+ 波蛮牛权重
}  // namespace wave

// ---- 武器（docs/03 4.6 数值基线；T4 先落地手枪）----
namespace weapon {
constexpr float kPistolFireInterval = 1.0f / 2.4f;  // 攻击间隔（秒）≈0.4167，即射速 2.4 发/秒
constexpr float kPistolDamage       = 10.0f;        // 单发伤害（爬行者 20 HP 恰为两枪）
constexpr float kPistolRange        = 600.0f;       // 索敌射程（px）：只锁定射程内最近敌人
constexpr float kMuzzleOffset       = 20.0f;        // 枪口偏移：玩家圆心外移（半径 14 + 余量）

// ---- 霰弹枪（T9；docs/03 4.3：近距离高爆发、高风险定位）----
constexpr float kShotgunFireInterval = 1.0f / 0.8f;  // 射速 0.8 发/秒（间隔 1.25s，比手枪慢 3 倍）
constexpr float kShotgunDamage       = 12.0f;        // 每颗弹丸伤害（贴脸 5 发全中 60）
constexpr float kShotgunRange        = 500.0f;       // 索敌与弹丸射程：只有近战射程，贴脸才全中
constexpr int   kShotgunPelletCount  = 5;            // 每次开火弹丸数
constexpr float kShotgunSpreadDeg    = 25.0f;        // 扇形总开角（±12.5° 均匀分布 5 发）

// ---- 环绕飞刃（T9；docs/03 4.3：被动近身威慑，不参与索敌）----
constexpr float kOrbitalBladeDamage  = 15.0f;  // 单刃单次命中伤害（每敌 0.5s 至多一次 ≈ 30 DPS/刃）
constexpr float kOrbitalBladeRadius  = 10.0f;  // 刃体碰撞半径（命中判定与绘制）
constexpr float kOrbitalRadius       = 90.0f;  // 环绕半径（px）：略大于玩家贴身层
constexpr float kOrbitalAngularSpeed = 1.8f;   // 旋转角速度（rad/s）：2s 内旋转一圈
constexpr float kOrbitalHitInterval  = 0.5f;   // 同一敌人的伤害结算冷却（秒）
constexpr int   kOrbitalBladeCount   = 2;      // 初始刃数（与任务规格一致的两刃）
constexpr Color kOrbitalBladeColor   {240, 240, 255, 255};  // 刃体：银白
constexpr Color kOrbitalBladeGlow    {140, 160, 255, 90};    // 刃体光晕：淡蓝（提示杀伤范围）

// ---- 武器已有再选 → 升级（docs/03 4.7；差异化数值依据 docs/05 平衡自查 §4.1）----
constexpr float kWeaponUpgradeDamage   = 6.0f;  // 手枪等单发类：再选单发伤害 +6（≈+25%）
constexpr float kShotgunUpgradeDamage  = 3.0f;  // 霰弹：每弹丸 +3（5 发齐射 +15，≈+25%）
constexpr float kOrbitalUpgradeDamage  = 4.0f;  // 飞刃：单刃 +4（≈+27%）
                                           // 理由：若共用 +6，霰弹齐射增伤 +50%、飞刃 +40%，
                                           // 装备类词条复选时强弱武器成长失衡（docs/05）
}  // namespace weapon

// ---- 子弹（docs/03 4.6：弹速 600、射程 900）----
namespace projectile {
constexpr float kBulletRadius   = 4.0f;    // 子弹碰撞半径
constexpr float kBulletSpeed    = 600.0f;  // 弹速（px/s），方向在开火时归一化
constexpr float kBulletMaxRange = 900.0f;  // 最大射程（px），超出自动消失
// ---- T13 程序化弹体配色（T12 贴图已废弃；绘制只用色值，碰撞尺寸 kBulletRadius 不变）----
constexpr Color kBulletColor      {255, 195, 64, 255};  // 玩家弹外圈（橙金）
constexpr Color kBulletCoreColor  {255, 245, 200, 255}; // 玩家弹内芯（亮白黄）
constexpr Color kBulletTrailColor {255, 140, 40, 150};  // 玩家弹尾部拖尾（半透明橙）
constexpr Color kEnemyBulletColor {214, 58, 108, 255};  // 敌弹外圈（深红紫）
constexpr Color kEnemyBulletCoreColor {255, 168, 208, 255}; // 敌弹内芯（粉）
constexpr Color kEnemyBulletGlowColor {255, 90, 140, 60};   // 敌弹外围魔光（低透明）
constexpr float kBulletVisualSize = 26.0f;  // T13 弹体视觉长度（px）≈6.5×碰撞半径：醒目且不影响碰撞
}  // namespace projectile

// ---- 经验点（T5：击杀掉落视觉反馈；拾取与吸取 T7）----
namespace xp_orb {
constexpr float kSize = 12.0f;  // 绿色小方块边长（px）
constexpr float kVisualSize = 30.0f;  // T12 经验宝石视觉尺寸（px）≈2.5×边长：掉落醒目
constexpr float kPulseAmp   = 0.15f;  // T12 脉动呼吸幅度（±15% 缩放）
constexpr float kPulseSpeed = 4.0f;   // T12 脉动角速度（rad/s）
constexpr float kSpinSpeed  = 1.4f;   // T13 宝石慢速自转角速度（rad/s）
constexpr Color kGemDark    {30, 150, 104, 255};   // T13 程序化宝石主体暗面（青绿深）
constexpr Color kGemLight   {118, 244, 172, 255};  // 高光面（青绿亮）
constexpr Color kGemEdge    {226, 255, 240, 255};  // 描边（近白）
constexpr Color kGemGlow    {66, 210, 150, 44};    // 呼吸光晕（低透明青绿）
}  // namespace xp_orb

// ---- 升级（docs/03 4.5 经验曲线；T6 仅显示，升级逻辑 T7 迁 UpgradeSystem）----
namespace upgrade {
constexpr float kBaseExpNeeded = 20.0f;  // 升第 1 级所需经验
constexpr float kExpPerLevel   = 10.0f;  // 每级递增：升 L 级所需 = 20 + 10×(L-1)
}  // namespace upgrade

// ---- 词条数值（docs/03 4.7 词条池数值表；T7 词条 apply 引用，禁止散落硬编码）----
namespace affix {
constexpr float kDamageBoost       = 1.15f;  // 伤害强化：伤害 ×1.15
constexpr float kAttackSpeedBoost  = 1.10f;  // 急速：攻速 ×1.10
constexpr float kMoveSpeedBoost    = 1.08f;  // 疾跑：移速 ×1.08
constexpr float kHpBonus           = 20.0f;  // 强健：上限 +20 并立即回复 20
constexpr float kBulletSpeedBoost  = 1.25f;  // 高速子弹：弹速 ×1.25
constexpr float kPickupBonus       = 30.0f;  // 磁力：拾取半径 +30
constexpr float kHpRegenBonus      = 1.0f;   // 生命再生：每秒回复 1 HP
constexpr float kDamageTakenReduce = 0.9f;   // 韧性：受伤 ×0.9
// ---- T9 新增（docs/03 4.7 v2：词条池 17 项）----
constexpr float kLifestealPct      = 0.02f;  // 吸血：造成伤害的 2% 回复生命
constexpr int   kPenetrationBonus  = 1;      // 穿甲弹：子弹穿透数 +1（穿过 1 个敌人后消失）
constexpr float kAttackSpeedBig    = 1.20f;  // 神速：攻速 ×1.20（史诗；docs/05 §4.2 由 1.25 下调）
constexpr float kDamageTakenBig    = 0.80f;  // 铁甲：受伤 ×0.8（史诗）
constexpr float kHpBonusBig        = 40.0f;  // 巨人：上限 +40 并立即回复 20（史诗）
constexpr float kPickupBonusBig    = 60.0f;  // 强磁力：拾取半径 +60（史诗）
constexpr float kBladeDamageBonus  = 5.0f;   // 飞刃强化：单刃伤害 +5（需已有环绕飞刃）
}  // namespace affix

// ---- HUD（T6：屏幕空间 UI；颜色/字号保证深色背景下清晰）----
namespace ui {
constexpr const char* kFontPathYaHei  = "C:/Windows/Fonts/msyh.ttc";   // 中文字体：微软雅黑
constexpr const char* kFontPathSimHei = "C:/Windows/Fonts/simhei.ttf"; // 回退字体：黑体
// 中文字符集（供字体图集加载；后续任务新增 UI 文案时需在此追加对应汉字）
// T10 追加：主菜单/暂停/结算界面文案（开始游戏 竞技场幸存者 主菜单 统计 结束 操作说明 等）
constexpr const char* kCnCharSet =
    "第波剩余时间敌人击杀手枪自动射击死亡逻辑将在后续任务实现方向键移动刷怪退出·（）"
    "选择升级按或鼠标点击普通稀有史诗伤害强化攻健生命上限并回复高磁力拾取半径每秒"
    "受伤获得暂补偿霰环绕飞刃单已取本次再韧性急促跑武器「」下中一"
    "吸血神铁巨人甲弹穿透发散速子扇形：，"
    "开始游戏退出主菜单竞技场幸存者停继续重新返回本局统计结束你倒下等级活长分秒"
    "操作说明按回车确认总";
constexpr int   kFontBaseSize  = 32;   // 字体图集基准字号（px），绘制时按需缩放
constexpr int   kHudMargin     = 16;   // HUD 距屏幕边缘边距（px）
constexpr int   kExpBarWidth   = 260;  // 经验条宽（px）
constexpr int   kExpBarHeight  = 12;   // 经验条高（px）
constexpr int   kHpBarWidth    = 260;  // 血条宽（px）
constexpr int   kHpBarHeight   = 18;   // 血条高（px）
constexpr float kHpLerpSpeed   = 8.0f; // 血条平滑趋近速度（/s，指数趋近避免跳变）
constexpr Color kExpBarColor   {255, 200, 70, 255};  // 经验条强调（琥珀金；T13 填充改渐变，此色作亮端基准）
constexpr Color kBarBgColor    {20, 26, 40, 240};    // 条底（深蓝黑半透明）
constexpr Color kHpBarBgColor  {64, 24, 30, 240};    // 血条底（暗红半透明）
constexpr Color kHpFillColor   {74, 190, 92, 255};   // 血条填充（绿）
constexpr Color kTextMainColor {235, 235, 238, 255}; // 主文字（近白）
constexpr Color kTextDimColor  {168, 172, 186, 255}; // 次级文字（灰蓝）

// ---- T13 界面美化（仅绘制层配色；鼠标判定布局同源，数值不动）----
constexpr Color kPanelBg      {14, 20, 34, 214};   // HUD 面板底（半透明深蓝黑）
constexpr Color kPanelEdge    {94, 108, 140, 235}; // 面板描边（灰蓝）
constexpr Color kGoldAccent   {255, 200, 70, 255}; // 主色：琥珀金（等级/波次/击杀强调）
constexpr Color kBadgeInner   {36, 32, 22, 255};   // 等级菱形徽章内底
constexpr Color kExpFillTop   {255, 220, 104, 255};// 经验条渐变亮端
constexpr Color kExpFillBottom{226, 148, 30, 255}; // 经验条渐变暗端
constexpr Color kHpFillTop    {128, 232, 130, 255};// 血条渐变亮端
constexpr Color kHpFillBottom {30, 146, 66, 255};  // 血条渐变暗端
constexpr Color kEnemyTextRed {255, 122, 112, 255};// 敌人计数（红）
constexpr Color kKillTextGold {255, 214, 96, 255}; // 击杀计数（金）
constexpr Color kTextShadow   {0, 0, 0, 160};      // 标题投影
constexpr Color kCardEdgeCommon {166, 176, 190, 255}; // 普通卡描边
constexpr Color kCardEdgeRare   {112, 168, 255, 255}; // 稀有卡描边（蓝）
constexpr Color kCardEdgeEpic   {228, 178, 255, 255}; // 史诗卡描边（亮紫）
constexpr Color kChipBg       {0, 0, 0, 70};       // 稀有度标签底

// ---- T7 升级三选一卡片（布局与配色；布局统一由 UIManager::upgradeCardRect 提供）----
constexpr int   kCardWidth  = 340;  // 卡片宽（px）
constexpr int   kCardHeight = 180;  // 卡片高（px）
constexpr int   kCardGap    = 24;   // 卡片间距（px）
constexpr Color kDimOverlay {0, 0, 0, 150};          // 升级界面全屏遮罩（压暗世界）
constexpr Color kCardBgCommon   {52, 58, 70, 240};   // 普通卡底（白灰）
constexpr Color kCardBgRare     {36, 64, 124, 240};  // 稀有卡底（蓝）
constexpr Color kCardBgEpic     {96, 52, 142, 240};  // 史诗卡底（紫）
constexpr Color kCardTextCommon {225, 225, 225, 255};// 普通文字（近白）
constexpr Color kCardTextRare   {130, 175, 255, 255};// 稀有文字（亮蓝）
constexpr Color kCardTextEpic   {205, 140, 255, 255};// 史诗文字（亮紫）
constexpr Color kCardHoverBorder{250, 200, 60, 255}; // 悬停卡片描边（亮黄）

// ---- T10 通用菜单（主菜单/暂停/结算共用行式布局；键盘方向键+回车 / 鼠标点击）----
constexpr int   kMenuRowWidth  = 460;   // 菜单项行宽（水平居中列）
constexpr int   kMenuRowHeight = 58;    // 菜单项行高
constexpr int   kMenuRowGap    = 16;    // 菜单项行间距
constexpr float kMenuStartY    = 420.0f; // 第一项行顶 Y（逻辑坐标，位于标题下方）
constexpr Color kMenuTitleColor  {255, 210, 90, 255};   // 大标题（金）
constexpr Color kMenuSubColor    {150, 160, 175, 255};  // 副标题/说明文字（灰蓝）
constexpr Color kMenuBgNormal    {46, 52, 64, 255};     // 未选中项底色
constexpr Color kMenuBgHover     {92, 112, 168, 255};   // 选中项底色（高亮蓝）
constexpr Color kMenuBgHoverEdge {255, 220, 120, 255};  // 选中项描边（金）
constexpr Color kMenuTextColor   {232, 232, 232, 255};  // 菜单项文字（近白）
constexpr Color kGameOverRed     {255, 96, 72, 255};    // 结算标题（红）
}  // namespace ui

// ---- 空间哈希碰撞网格（T11；docs/03 2.8 落地）----
namespace collision {
constexpr int   kCellSize        = 160;  // 网格单元边长（px）：竞技场 3200×1800 → 20×11 格
constexpr float kMaxEntityRadius = 32.0f; // 邻域查询扩展半径：覆盖最大实体半径（蛮牛 28）+ 余量
}  // namespace collision

// ---- 打击感与性能日志（T11）----
namespace fx {
constexpr float kHitFlashDuration = 0.10f;   // 受击闪白时长（秒）
constexpr float kShakeAmpHurt     = 3.0f;    // 玩家受击轻微震动幅度（px）
constexpr float kShakeDurHurt     = 0.15f;   // 受击震动时长（秒）
constexpr float kShakeAmpBrute    = 7.0f;    // 蛮牛（精英）死亡震动幅度（px）
constexpr float kShakeDurBrute    = 0.35f;   // 蛮牛死亡震动时长（秒）
constexpr float kShakeAmpDeath    = 11.0f;   // 玩家死亡震动幅度（px，定格期间可见）
constexpr float kShakeDurDeath    = 0.50f;   // 玩家死亡震动时长（秒）
constexpr float kDeathFreezeTime  = 0.35f;   // 死亡定格时长（秒）：震动展示后进结算界面
constexpr int   kHitParticleCount = 5;       // 子弹命中粒子数
constexpr int   kKillParticleCount = 12;     // 击杀爆散粒子数
constexpr float kParticleSpeed    = 160.0f;  // 粒子初速（px/s）
constexpr float kParticleLife     = 0.25f;   // 粒子寿命（秒）
constexpr float kKillTextLife     = 0.8f;    // 击杀飘字存活（秒）
constexpr Color kSparkColor {255, 200, 90, 255};    // 命中火花（金黄）
constexpr Color kKillColor  {90, 230, 130, 255};    // 击杀爆散 + 飘字（绿）
constexpr int   kPerfLogEnemyThreshold = 40;        // 性能日志触发：场上敌人数 ≥ 该值才输出
}  // namespace fx

}  // namespace constants
