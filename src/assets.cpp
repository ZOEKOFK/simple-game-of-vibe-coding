// assets.cpp —— 全局素材集中管理实现（T12 美术资源替换；T13 子弹/宝石改程序化自绘）
#include "assets.h"

// 资源来源：角色 tile 出自 Kenney.nl「Tiny Dungeon」像素包（CC0 免费商用，16x16），
// 原包 License 见 assets/LICENSE_kenney_tiny_dungeon.txt；tile 自带深色底板，
// 已在入库前离线抠除（洪泛填充方式，避免误删同色描边/五官细节）。
// 环绕飞刃沿用「Top-down Shooter」素材（assets/LICENSE_kenney_topdown_shooter.txt）。
// 子弹/敌弹/经验宝石自 T13 起由 owner 实体程序化绘制，不再经本模块加载贴图。
#include <algorithm>  // std::max/min（透明边距裁剪包围盒计算）
#include <string>
#include <unordered_map>
#include <vector>

namespace assets {
namespace {

// ---- T13 路径稳健性修复 ----
// 素材此前一律走 "assets/sprites/x.png" 相对路径，依赖进程工作目录：
// 从 exe 目录启动（VS 调试 / 双击 exe，cwd=build\<config>）时找不到素材，
// 全部贴图加载失败 → 只剩阴影椭圆兜底（曾被视为"角色变黑块"的元凶）。
// 修复：按候选顺序探测资源根，首个 FileExists 命中者为准：
//   1. exe 同目录（资源随 exe 布置）
//   2. exe 上两级（VS 输出布局 build\<config> → 项目根）
//   3. 当前工作目录相对路径（原始行为，兼容从项目根启动）
std::string resolvePath(const std::string& relPath) {
    const std::string exeDir = GetApplicationDirectory();  // 以 '\' 结尾的绝对路径
    const std::vector<std::string> cands = {
        exeDir + relPath,
        exeDir + "..\\..\\" + relPath,
        relPath,
    };
    for (const auto& c : cands) {
        if (FileExists(c.c_str())) return c;
    }
    return relPath;  // 全部未命中：返回原始相对路径（LoadTexture 自身含 exe 目录回退）
}

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
    Texture2D tex = LoadTexture(resolvePath(key).c_str());

    // T13 透明包围盒裁剪：角色 tile 取自 16x16 tileset，有效内容只占其中一小块
    // 且不居中（直接绘制会整体偏小偏位）。加载后按 alpha 阈值裁掉透明边距，
    // 让贴图内容填满绘制框、视觉中心对齐实体中心（与 T12 飞刃同一处理）。
    // 两类差别：tile 是硬边像素画 → margin 0 硬切；飞刃带羽化边 → margin 4 保过渡。
    const bool tileSprite = std::string(relPath) == "player.png" ||
                            std::string(relPath) == "enemy_crawler.png" ||
                            std::string(relPath) == "enemy_spitter.png" ||
                            std::string(relPath) == "enemy_brute.png";
    const bool cropItem = tileSprite || std::string(relPath) == "orbital_blade.png";
    if (cropItem && tex.id != 0) {
        Image img = LoadImageFromTexture(tex);
        UnloadTexture(tex);
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);  // 归一化格式便于逐像素扫描
        const int w = img.width, h = img.height;
        const unsigned char* px = static_cast<unsigned char*>(img.data);
        int minX = w, minY = h, maxX = -1, maxY = -1;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (px[(y * w + x) * 4 + 3] > 8) {  // alpha 阈值：跳过微弱噪点
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
        if (maxX >= minX && maxY >= minY) {
            const int kMargin = tileSprite ? 0 : 4;  // tile 硬切；飞刃保留羽化边距
            const int left = std::max(0, minX - kMargin);
            const int top = std::max(0, minY - kMargin);
            const int right = std::min(w, maxX + kMargin + 1);
            const int bottom = std::min(h, maxY + kMargin + 1);
            ImageCrop(&img, Rectangle{static_cast<float>(left), static_cast<float>(top),
                                      static_cast<float>(right - left),
                                      static_cast<float>(bottom - top)});
        }
        tex = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    // T13 像素风锐利：全部贴图用 POINT 采样（角色 tile 放大 2~3 倍时保持锯齿块面感）
    if (tex.id != 0) SetTextureFilter(tex, TEXTURE_FILTER_POINT);

    texs.emplace(key, tex);
    return tex;
}

}  // namespace

Texture2D player()          { return load("player.png"); }
Texture2D enemyCrawler()    { return load("enemy_crawler.png"); }
Texture2D enemySpitter()    { return load("enemy_spitter.png"); }
Texture2D enemyBrute()      { return load("enemy_brute.png"); }
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
