// assets.cpp —— 全局素材集中管理实现（T12 美术资源替换）
#include "assets.h"

// 资源来源：Kenney.nl「Top-down Shooter」与「Particle Pack」（均 CC0 免费商用），
// 原始下载与许可证见 assets/LICENSE_kenney_topdown_shooter.txt 与
// assets/LICENSE_kenney_particle_pack.txt。下载后仅保留本项目使用的贴图至 assets/sprites/。
#include <string>
#include <unordered_map>

namespace assets {
namespace {

// 惰性加载：首次访问时 LoadTexture 并缓存（raylib 内部按路径缓存），
// 后续同一路径直接返回缓存纹理；退出时统一 unloadAll() 释放。
std::unordered_map<std::string, Texture2D>& registry() {
    static std::unordered_map<std::string, Texture2D> texs;
    return texs;
}

Texture2D load(const char* relPath) {
    const std::string key = std::string("assets/sprites/") + relPath;
    auto& texs = registry();
    auto it = texs.find(key);
    if (it != texs.end()) return it->second;  // 已加载：直接返回
    Texture2D tex = LoadTexture(key.c_str());
    texs.emplace(key, tex);
    return tex;
}

}  // namespace

Texture2D player()          { return load("player.png"); }
Texture2D enemyCrawler()    { return load("enemy_crawler.png"); }
Texture2D enemySpitter()    { return load("enemy_spitter.png"); }
Texture2D enemyBrute()      { return load("enemy_brute.png"); }
Texture2D bullet()          { return load("bullet.png"); }
Texture2D enemyBullet()     { return load("enemy_bullet.png"); }
Texture2D xpGem()           { return load("xp_gem.png"); }
Texture2D orbitalBlade()    { return load("orbital_blade.png"); }

void drawCentered(Texture2D tex, Vector2 pos, float worldSize,
                  float rotationDeg, Color tint) {
    if (tex.id == 0) return;  // 纹理加载失败时静默跳过（几何圆绘制由各调用方保留兜底）
    const float aspect = static_cast<float>(tex.height) / static_cast<float>(tex.width);
    const float w = worldSize;
    const float h = worldSize * aspect;
    const Rectangle src{0.0f, 0.0f, static_cast<float>(tex.width),
                        static_cast<float>(tex.height)};
    const Rectangle dest{pos.x, pos.y, w, h};
    const Vector2 origin{w * 0.5f, h * 0.5f};
    DrawTexturePro(tex, src, dest, origin, rotationDeg, tint);
}

void unloadAll() {
    for (auto& kv : registry()) UnloadTexture(kv.second);
    registry().clear();
}

}  // namespace assets
