// ui.cpp —— UIManager 实现（T6：进行中游戏 HUD）
#include "ui.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "constants.h"
#include "player.h"  // Player 完整类型：HP/武器查询
#include "weapon.h"  // Weapon 展示接口

namespace {

// 数值钳制 [0,1]（条填充比例用）
float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

// ---- T7 升级卡片：稀有度配色与标签（普通白 / 稀有蓝 / 史诗紫）----
Color cardBg(Rarity r) {
    switch (r) {
        case Rarity::Rare: return constants::ui::kCardBgRare;
        case Rarity::Epic: return constants::ui::kCardBgEpic;
        default:           return constants::ui::kCardBgCommon;
    }
}
Color cardText(Rarity r) {
    switch (r) {
        case Rarity::Rare: return constants::ui::kCardTextRare;
        case Rarity::Epic: return constants::ui::kCardTextEpic;
        default:           return constants::ui::kCardTextCommon;
    }
}
const char* rarityLabel(Rarity r, bool cn) {
    switch (r) {
        case Rarity::Rare: return cn ? "稀有" : "Rare";
        case Rarity::Epic: return cn ? "史诗" : "Epic";
        default:           return cn ? "普通" : "Common";
    }
}

// 构建字体图集 codepoints：ASCII(32~126) + 常用中文集合（constants::ui::kCnCharSet）
std::vector<int> buildCodepoints() {
    std::vector<int> cps;
    cps.reserve(128);
    for (int c = 32; c <= 126; ++c) cps.push_back(c);  // 数字/字母/常见标点

    int cnCount = 0;
    int* cnCps = LoadCodepoints(constants::ui::kCnCharSet, &cnCount);  // UTF-8 → codepoints
    if (cnCps != nullptr) {
        for (int i = 0; i < cnCount; ++i) cps.push_back(cnCps[i]);
        UnloadCodepoints(cnCps);
    }
    return cps;
}

}  // namespace

UIManager::~UIManager() {
    if (font_.texture.id != 0) UnloadFont(font_);  // 兜底（常规路径由 unload() 处理）
}

void UIManager::load() {
    // 中文字体方案：优先微软雅黑（ttc 第 0 面），失败退黑体，再失败回退默认字体+英文文案
    // （默认字体不含 CJK 字形，直接画中文会乱码，故必须用系统字体 + 字符集图集）
    std::vector<int> cps = buildCodepoints();  // 非 const：LoadFontEx 需要 int*
    font_ = LoadFontEx(constants::ui::kFontPathYaHei, constants::ui::kFontBaseSize,
                       cps.data(), static_cast<int>(cps.size()));
    if (font_.texture.id == 0) {
        font_ = LoadFontEx(constants::ui::kFontPathSimHei, constants::ui::kFontBaseSize,
                           cps.data(), static_cast<int>(cps.size()));
    }
    fontValid_ = font_.texture.id != 0;
}

void UIManager::unload() {
    if (font_.texture.id != 0) {
        UnloadFont(font_);
        font_ = Font{};
    }
    fontValid_ = false;
}

Vector2 UIManager::measureText(const char* text, float size) const {
    if (fontValid_) return MeasureTextEx(font_, text, size, 1.0f);
    // 默认字体回退测量（高度近似取字号）
    return Vector2{static_cast<float>(MeasureText(text, static_cast<int>(size))), size};
}

void UIManager::drawTextAligned(const char* text, float x, float y, float size,
                                Color color, int align) const {
    const Vector2 m = measureText(text, size);
    if (align == 1) x -= m.x * 0.5f;            // 水平居中
    else if (align == 2) x -= m.x;              // 右对齐
    if (fontValid_) {
        DrawTextEx(font_, text, Vector2{x, y}, size, 1.0f, color);
    } else {
        DrawText(text, static_cast<int>(x), static_cast<int>(y),
                 static_cast<int>(size), color);
    }
}

void UIManager::drawText(const char* text, float x, float y, float size, Color color) const {
    drawTextAligned(text, x, y, size, color, 0);
}

void UIManager::drawCenterNotice(const char* text, float yOffset, float size, Color color) const {
    const float y = constants::window::kLogicHeight * 0.5f + yOffset - size * 0.5f;
    drawTextAligned(text, constants::window::kLogicWidth * 0.5f, y, size, color, 1);
}

void UIManager::update(float dt, const Player& player) {
    // 血条平滑过渡：显示值以指数速度趋近实际 HP（受伤时缓慢回落而非跳变）
    const float target = static_cast<float>(player.hp());
    if (displayHp_ < 0.0f) displayHp_ = target;  // 首帧直接对齐，避免开场动画
    const float t = 1.0f - std::exp(-constants::ui::kHpLerpSpeed * dt);
    displayHp_ += (target - displayHp_) * t;
}

void UIManager::drawHUD(const HudSnapshot& s) const {
    const int m = constants::ui::kHudMargin;  // 边缘边距
    const int W = constants::window::kLogicWidth;
    if (s.player == nullptr) return;

    // ---- 左上：等级 + 经验条 + 血条 ----
    const std::string lvText = "Lv." + std::to_string(s.level);
    drawText(lvText.c_str(), static_cast<float>(m), static_cast<float>(m) - 4.0f, 22,
             constants::ui::kExpBarColor);

    // 经验条（黄）：填充比例 = 当前经验 / 升级所需
    const float expX = static_cast<float>(m);
    const float expY = static_cast<float>(m) + 30.0f;
    const float expPct = s.expToNext > 0.0f ? s.exp / s.expToNext : 0.0f;
    DrawRectangle(static_cast<int>(expX), static_cast<int>(expY),
                  constants::ui::kExpBarWidth, constants::ui::kExpBarHeight,
                  constants::ui::kBarBgColor);
    DrawRectangle(static_cast<int>(expX), static_cast<int>(expY),
                  static_cast<int>(constants::ui::kExpBarWidth * clamp01(expPct)),
                  constants::ui::kExpBarHeight, constants::ui::kExpBarColor);
    const std::string expText =
        std::to_string(s.exp) + " / " + std::to_string(static_cast<int>(s.expToNext));
    drawText(expText.c_str(), expX + constants::ui::kExpBarWidth + 8.0f, expY - 2.0f, 16,
             constants::ui::kTextDimColor);

    // 血条（红底绿条）：宽度按平滑显示值比例；数字显示实际 HP
    const float hpX = static_cast<float>(m);
    const float hpY = expY + constants::ui::kExpBarHeight + 8.0f;
    const float maxHp = static_cast<float>(s.player->maxHp());
    const float hpPct = maxHp > 0.0f ? displayHp_ / maxHp : 0.0f;
    DrawRectangle(static_cast<int>(hpX), static_cast<int>(hpY),
                  constants::ui::kHpBarWidth, constants::ui::kHpBarHeight,
                  constants::ui::kHpBarBgColor);
    DrawRectangle(static_cast<int>(hpX), static_cast<int>(hpY),
                  static_cast<int>(constants::ui::kHpBarWidth * clamp01(hpPct)),
                  constants::ui::kHpBarHeight, constants::ui::kHpFillColor);
    const std::string hpText = std::to_string(s.player->hp()) + " / " +
                               std::to_string(s.player->maxHp());
    drawTextAligned(hpText.c_str(), hpX + constants::ui::kHpBarWidth * 0.5f,
                    hpY + 1.0f, 16, constants::ui::kTextMainColor, 1);

    // ---- 右上：第 X 波 / 波次状态行（T8）/ 敌人 / 击杀 ----
    const float rightX = static_cast<float>(W - m);
    const std::string waveText = fontValid_ ? ("第 " + std::to_string(s.wave) + " 波")
                                            : ("Wave " + std::to_string(s.wave));
    drawTextAligned(waveText.c_str(), rightX, static_cast<float>(m) - 4.0f, 20,
                    constants::ui::kTextMainColor, 2);

    // T8 状态行：0 波间倒计时（3,2,1）/ 1 刷怪中剩余投放 / 2 清场剩余敌人
    std::string stateText;
    Color stateColor = constants::ui::kTextDimColor;
    if (s.waveState == 0) {
        const int countdown = static_cast<int>(s.restTimeLeft) + 1;  // 3.0→3、0.01→1
        stateText = fontValid_
            ? ("下一波 " + std::to_string(s.wave) + ": " + std::to_string(countdown))
            : ("Next wave " + std::to_string(s.wave) + ": " + std::to_string(countdown));
        stateColor = constants::ui::kExpBarColor;  // 黄色：提示即将开刷
    } else if (s.waveState == 1) {
        stateText = fontValid_ ? ("刷怪中 剩余 " + std::to_string(s.remainingToSpawn))
                               : ("Spawning: " + std::to_string(s.remainingToSpawn));
    } else {
        stateText = fontValid_ ? ("剩余敌人 " + std::to_string(s.aliveInWave))
                               : ("Enemies left: " + std::to_string(s.aliveInWave));
    }
    drawTextAligned(stateText.c_str(), rightX, static_cast<float>(m) + 30.0f, 16,
                    stateColor, 2);
    const std::string enemyText = fontValid_ ? ("敌人: " + std::to_string(s.enemyCount))
                                             : ("Enemies: " + std::to_string(s.enemyCount));
    drawTextAligned(enemyText.c_str(), rightX, static_cast<float>(m) + 52.0f, 16, GOLD, 2);
    const std::string killText = fontValid_ ? ("击杀: " + std::to_string(s.killCount))
                                            : ("Kills: " + std::to_string(s.killCount));
    drawTextAligned(killText.c_str(), rightX, static_cast<float>(m) + 74.0f, 16,
                    constants::ui::kTextMainColor, 2);

    // ---- 底部居中：本局武器列表（T9：多武器逐个拼接"名 · 说明"，字体回退时拼注册名）----
    const auto& weapons = s.player->weapons();
    if (!weapons.empty()) {
        std::string label;
        for (size_t i = 0; i < weapons.size(); ++i) {
            if (i > 0) label += "    ";  // 多武器四空格间隔，便于区分
            if (fontValid_) {
                label += weapons[i]->displayName();
                label += " · ";
                label += weapons[i]->description();
            } else {
                label += weapons[i]->typeId();
            }
        }
        drawTextAligned(label.c_str(), W * 0.5f,
                        constants::window::kLogicHeight - m - 26.0f, 20,
                        constants::ui::kTextMainColor, 1);
    }
}

// ============================================================
// T7：升级三选一（UPGRADE 状态）
// ============================================================

Rectangle UIManager::upgradeCardRect(int index) const {
    // 三卡水平排列居中：总宽 = 3×卡宽 + 2×间距；布局同时供绘制与鼠标判定，
    // 由 GameEngine 借助本函数判定鼠标命中，保证绘制/判定零错位
    const float totalW = 3.0f * constants::ui::kCardWidth +
                         2.0f * constants::ui::kCardGap;
    const float x0 = (constants::window::kLogicWidth - totalW) * 0.5f;
    const float y0 = (constants::window::kLogicHeight - constants::ui::kCardHeight) * 0.5f;
    return Rectangle{x0 + index * (constants::ui::kCardWidth + constants::ui::kCardGap),
                     y0,
                     static_cast<float>(constants::ui::kCardWidth),
                     static_cast<float>(constants::ui::kCardHeight)};
}

void UIManager::drawUpgradeChoices(const std::vector<Affix>& choices, int hovered,
                                   const Player* player) const {
    const float cx = constants::window::kLogicWidth * 0.5f;  // 水平中轴
    const Rectangle first = upgradeCardRect(0);

    // 标题（卡片上方）与操作提示（卡片下方）；字体不可用整体回退英文
    drawTextAligned(fontValid_ ? "选择升级" : "LEVEL UP", cx,
                    first.y - 64.0f, 32, constants::ui::kTextMainColor, 1);
    drawTextAligned(fontValid_ ? "按 1 / 2 / 3 或鼠标点击选择"
                               : "Press 1 / 2 / 3 or click to choose",
                    cx, first.y + constants::ui::kCardHeight + 24.0f, 18,
                    constants::ui::kTextDimColor, 1);

    for (size_t i = 0; i < choices.size() && i < 3; ++i) {
        const Affix& a = choices[i];
        const Rectangle r = upgradeCardRect(static_cast<int>(i));
        const Color fg = cardText(a.rarity);

        // 卡片底（稀有度配色）+ 悬停亮黄描边
        DrawRectangleRounded(r, 0.06f, 12, cardBg(a.rarity));
        if (static_cast<int>(i) == hovered) {
            DrawRectangleLinesEx(r, 3.0f, constants::ui::kCardHoverBorder);
        }

        // T9 装备类动态文案：已持有该武器 → 展示"升级武器：伤害 +N"（N 取自
        // upgradeDamageAmount()，与真实升级增量一致；不同武器 N 不同，见 docs/05 §4.1）
        std::string ownedDesc;
        const char* name = fontValid_ ? a.name.c_str() : a.nameEn.c_str();
        const char* desc = fontValid_ ? a.description.c_str() : a.descriptionEn.c_str();
        if (a.kind == AffixKind::Weapon && player != nullptr) {
            const auto& ws = player->weapons();
            const auto it = std::find_if(
                ws.begin(), ws.end(),
                [&](const auto& w) { return w && w->typeId() == a.weaponId; });
            if (it != ws.end()) {
                const int amt = static_cast<int>((*it)->upgradeDamageAmount());
                ownedDesc = fontValid_ ? ("升级武器：伤害 +" + std::to_string(amt))
                                       : ("Weapon upgrade: damage +" + std::to_string(amt));
                desc = ownedDesc.c_str();
            }
        }
        drawText(name, r.x + 20.0f, r.y + 16.0f, 22, fg);                       // 词条名
        drawText(rarityLabel(a.rarity, fontValid_), r.x + 20.0f, r.y + 48.0f,   // 稀有度标签
                 14, fg);
        drawText(desc, r.x + 20.0f, r.y + 84.0f, 16,                            // 效果描述
                 constants::ui::kCardTextCommon);

        // 快捷键标注（右下角 [n]，键盘 1/2/3 与单击均可选）
        const std::string key = "[" + std::to_string(i + 1) + "]";
        drawTextAligned(key.c_str(), r.x + r.width - 20.0f, r.y + r.height - 34.0f,
                        20, constants::ui::kCardTextCommon, 2);
    }
}