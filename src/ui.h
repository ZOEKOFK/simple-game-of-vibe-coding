#pragma once
// ============================================================
// ui.h —— UIManager：进行中游戏的 HUD
// T6：左上 等级/经验条/血条（平滑过渡）；右上 波次/计时占位/敌人/击杀；
//     底部居中 当前武器说明。全部屏幕空间绘制（不随镜头移动）。
//     升级三选一/菜单/结算等界面在 T7/T10 扩展（docs/03 类图预留）。
// ============================================================
#include "raylib.h"

class Player;

// HUD 快照：每帧由 GameEngine 收集数值后传入，UI 与游戏逻辑解耦
struct HudSnapshot {
    const Player* player = nullptr;  // HP/武器信息源
    int   enemyCount = 0;            // 场上敌人数量
    int   wave = 1;                  // 当前波次（T6 占位显示，T8 波次系统接管）
    float waveTimer = 0.0f;          // 波次计时（T6 占位 0，T8 接管）
    int   killCount = 0;             // 累计击杀数
    int   level = 1;                 // 玩家等级（T6 占位，经验逻辑 T7 迁 UpgradeSystem）
    int   exp = 0;                   // 当前等级内经验
    float expToNext = 1.0f;          // 升下一级所需经验（docs/03 4.5 公式）
};

class UIManager {
public:
    UIManager() = default;
    ~UIManager();
    UIManager(const UIManager&) = delete;             // Font 持纹理资源，禁拷贝
    UIManager& operator=(const UIManager&) = delete;

    void load();   // 窗口（GL 上下文）就绪后调用：加载中文字体，失败回退默认字体+英文文案
    void unload(); // 窗口关闭前调用：释放字体资源
    bool cnFontAvailable() const { return fontValid_; }  // 供文案回退判断（true=可画中文）

    void update(float dt, const Player& player);   // 血条显示值平滑趋近（防跳变）
    void drawHUD(const HudSnapshot& s) const;      // PLAYING 状态 HUD（屏幕空间）

    // ---- 公共文字绘制（带中文字体；供 GameEngine 画操作提示/死亡占位等）----
    void drawText(const char* text, float x, float y, float size, Color color) const;
    void drawCenterNotice(const char* text, float yOffset, float size, Color color) const;

private:
    bool fontValid_ = false;       // 中文字体加载成功标志（false 时文案整体回退英文）
    Font font_{};                  // 中文字体（微软雅黑，失败退黑体）
    float displayHp_ = -1.0f;      // 血条平滑显示值（<0 未初始化，首帧直接对齐实际值）

    // 内部辅助：统一走 fontValid_ 选择 DrawTextEx / DrawText
    Vector2 measureText(const char* text, float size) const;
    void drawTextAligned(const char* text, float x, float y, float size,
                         Color color, int align) const;  // align: 0 左 / 1 中 / 2 右
};