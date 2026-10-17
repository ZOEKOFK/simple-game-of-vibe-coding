#pragma once
// ============================================================
// ui.h —— UIManager：进行中游戏的 HUD
// T6：左上 等级/经验条/血条（平滑过渡）；右上 波次/计时占位/敌人/击杀；
//     底部居中 当前武器说明。全部屏幕空间绘制（不随镜头移动）。
// T7：升级三选一卡片（稀有度配色 + 键盘 1/2/3 或鼠标点击 + 悬停描边）。
// T8：右上波次状态行（波间倒计时"下一波 X: 3"/刷怪中剩余/清场剩余敌人）。
//     菜单/结算等界面在 T10 扩展（docs/03 类图预留）。
// ============================================================
#include <vector>

#include "affix_library.h"  // Affix：T7 升级卡片数据（名称/描述/稀有度）
#include "raylib.h"

class Player;

// HUD 快照：每帧由 GameEngine 收集数值后传入，UI 与游戏逻辑解耦
struct HudSnapshot {
    const Player* player = nullptr;  // HP/武器信息源
    int   enemyCount = 0;            // 场上敌人数量
    int   wave = 1;                  // 当前波号（T8：WAITING 期间即"下一波"波号）
    int   waveState = 0;             // T8 波次状态：0 波间倒计时 / 1 刷怪中 / 2 清场战斗
    float restTimeLeft = 1.0f;       // T8 波间剩余倒计时（秒，显示 3/2/1）
    int   remainingToSpawn = 0;      // T8 本波剩余待投放（SPAWNING 状态显示）
    int   aliveInWave = 0;           // T8 本波剩余存活（FIGHTING/刷怪后显示）
    int   killCount = 0;             // 累计击杀数
    int   level = 1;                 // 玩家等级（T7 起由 UpgradeSystem 提供）
    int   exp = 0;                   // 当前等级内经验
    float expToNext = 1.0f;          // 升下一级所需经验（docs/03 4.5 公式）
};

// T10 结算统计：死亡/重开瞬间由 GameEngine 汇总传入，UI 只读展示
struct EndScreenStats {
    int   wavesSurvived = 0;   // 存活波次（已完成波数 = 当前波号 - 1）
    int   killCount = 0;       // 总击杀
    int   level = 1;           // 最终等级
    float elapsedSec = 0.0f;   // 存活时长（秒）
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
    void resetHud();                               // T10：重开局血条平滑值复位（防残留旧值）

    // ---- T10 菜单界面（主菜单/暂停/结算；行式布局 + 键盘/鼠标）----
    Rectangle menuRowRect(int index) const;        // 第 index 行菜单项区（逻辑坐标，供鼠标判定）
    void drawMainMenu(int cursor) const;           // 标题 + 开始/退出
    void drawPauseMenu(int cursor) const;          // 已暂停 + 继续/回主菜单/退出
    void drawGameOverScreen(const EndScreenStats& s, int cursor) const;  // 统计 + 重开/回主菜单

    // ---- T7 升级三选一（UPGRADE 状态；遮罩由 GameEngine 绘制）----
    Rectangle upgradeCardRect(int index) const;    // 第 index(0~2) 张卡片区（逻辑坐标，供鼠标判定）
    // T9：player 供"已持有装备词条"动态文案（升级伤害）判定；nullptr 退化为静态文案
    void drawUpgradeChoices(const std::vector<Affix>& choices, int hovered,
                            const Player* player) const;

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
    // T10：绘制一组居中菜单项（选中行高亮描边），布局与 menuRowRect 同源
    void drawMenuRows(const std::vector<std::string>& items, int cursor) const;
};