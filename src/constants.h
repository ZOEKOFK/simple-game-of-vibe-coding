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

// 生成规则（T3 调试用，正式波次 T8 接入）
constexpr int   kDebugSpawnCount    = 10;     // F1 一次刷怪数量
constexpr float kSpawnMinPlayerDist = 400.0f; // 出生点距玩家最小距离，防骑脸
constexpr float kSpawnOutsideDepth  = 36.0f;  // 墙外出生：距墙外缘的深度（px），敌人从场外入场

// 配色：红色系与玩家蓝色明显区分；接触中描边变亮黄作为接触标记
constexpr Color kCrawlerColor        {220, 60, 50, 255};
constexpr Color kCrawlerOutlineColor {110, 30, 25, 255};
constexpr Color kContactColor        {255, 210, 90, 255};
}  // namespace enemy

}  // namespace constants
