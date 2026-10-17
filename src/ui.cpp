// ui.cpp —— UIManager 实现（T6：进行中游戏 HUD）
#include "ui.h"

#include <cmath>
#include <string>
#include <vector>

#include "constants.h"
#include "player.h"  // Player 完整类型：HP/武器查询
#include "weapon.h"  // Weapon 展示接口

namespace {

// 数值钳制 [0,1]（条填充比例用）
float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

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

    // ---- 右上：波次 / 剩余时间（占位 0）/ 敌人 / 击杀 ----
    const float rightX = static_cast<float>(W - m);
    const std::string waveText = fontValid_ ? ("第 " + std::to_string(s.wave) + " 波")
                                            : ("Wave " + std::to_string(s.wave));
    drawTextAligned(waveText.c_str(), rightX, static_cast<float>(m) - 4.0f, 20,
                    constants::ui::kTextMainColor, 2);
    const std::string timeText = fontValid_
        ? ("剩余时间: " + std::to_string(static_cast<int>(s.waveTimer)))
        : ("Time: " + std::to_string(static_cast<int>(s.waveTimer)));  // 占位 0（T8 接管）
    drawTextAligned(timeText.c_str(), rightX, static_cast<float>(m) + 30.0f, 16,
                    constants::ui::kTextDimColor, 2);
    const std::string enemyText = fontValid_ ? ("敌人: " + std::to_string(s.enemyCount))
                                             : ("Enemies: " + std::to_string(s.enemyCount));
    drawTextAligned(enemyText.c_str(), rightX, static_cast<float>(m) + 52.0f, 16, GOLD, 2);
    const std::string killText = fontValid_ ? ("击杀: " + std::to_string(s.killCount))
                                            : ("Kills: " + std::to_string(s.killCount));
    drawTextAligned(killText.c_str(), rightX, static_cast<float>(m) + 74.0f, 16,
                    constants::ui::kTextMainColor, 2);

    // ---- 底部居中：当前武器名 + 简要说明（如"手枪 · 自动射击"）----
    const auto& weapons = s.player->weapons();
    if (!weapons.empty()) {
        const Weapon& w = *weapons.front();
        // 字体回退模式下描述为中文会乱码，仅显示注册名（pistol）
        const std::string label = fontValid_
            ? (std::string(w.displayName()) + " · " + w.description())
            : w.typeId();
        drawTextAligned(label.c_str(), W * 0.5f,
                        constants::window::kLogicHeight - m - 26.0f, 20,
                        constants::ui::kTextMainColor, 1);
    }
}