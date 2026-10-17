#pragma once
// ============================================================
// effects.h —— 打击感表现系统（T11）
// 内容：命中/击杀粒子、击杀数字飘字、屏幕轻微震动。
// 粒子与飘字为世界坐标实体（在摄像机块内绘制）；震动为时间驱动的摄像机偏移，
// 死亡定格期间仍会衰减可见（不依赖世界 update）。
// 依赖：raylib 绘图/随机/计时；无游戏逻辑依赖，UI 可安全移除。
// ============================================================
#include <string>
#include <vector>

#include "raylib.h"  // Vector2 / Color / 绘图与随机

struct Particle {
    Vector2 pos{};          // 世界坐标
    Vector2 vel{};          // 速度（px/s）
    float   life = 0.0f;    // 剩余寿命（秒）
    float   maxLife = 1.0f; // 初始寿命（衰减比例 = life/maxLife）
    float   size = 3.0f;    // 绘制半径（px）
    Color   color{255, 255, 255, 255};
};

struct KillText {
    Vector2 pos{};          // 世界坐标（击杀点）
    std::string text{};     // 如 "+5"（ASCII，默认字体可绘）
    float life = 0.0f;      // 剩余寿命（秒）
    float maxLife = 1.0f;
    Color color{255, 255, 255, 255};
};

class Effects {
public:
    void update(float dt);                    // 粒子/飘字寿命推进（PLAYING 每固定步调用）
    void draw() const;                        // 世界坐标绘制（BeginMode2D 块内调用）

    void spawnBurst(const Vector2& pos, int count, Color color,
                    float speed, float life); // 粒子爆散（命中/击杀）
    void spawnKillText(const Vector2& pos, const std::string& text,
                       Color color);          // 击杀数字飘字（上升渐隐）

    void addShake(float amplitude, float duration);   // 触发屏幕震动
    Vector2 shakeOffset() const;              // 当前帧摄像机偏移（无震动/已结束返回 0）

    void clear();                             // 重开/切场景清空全部表现残留

private:
    std::vector<Particle> particles_{};
    std::vector<KillText> texts_{};
    double shakeStart_ = 0.0;                 // 震动起始时刻（GetTime，秒）
    float  shakeAmp_ = 0.0f;                  // 震动幅度（px）
    float  shakeDur_ = 0.0f;                  // 震动时长（秒）
};
