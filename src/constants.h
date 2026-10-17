#pragma once
// ============================================================
// constants.h —— 全局常量集中定义（分模块命名空间）
// 约定：所有数值基线集中于此，代码中禁止散落硬编码。
// T1 只定义窗口与主循环常量；玩法数值基线随后续任务补充。
// ============================================================

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

}  // namespace constants
