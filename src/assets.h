#pragma once
// ============================================================
// assets.h —— 全局素材集中管理（T12 美术资源替换）
// 从 Kenney.nl 下载的 CC0 素材（Top-down Shooter + Particle Pack，
// 见 assets/LICENSE_*.txt）替代原先的几何图形绘制。
// 约定：所有贴图经本模块惰性加载并缓存，绘制方只查询不管理生命周期；
//       程序退出前统一调用 assets::unloadAll() 释放。
// ============================================================
#include "raylib.h"

namespace assets {

// ---- 贴图访问（惰性加载，首次调用 LoadTexture 并缓存）----
Texture2D player();          // 玩家（Soldier 1）
Texture2D enemyCrawler();    // 爬行者（Zombie 1）
Texture2D enemySpitter();    // 喷吐者（Man Blue）
Texture2D enemyBrute();      // 蛮牛精英（Robot 1）
Texture2D bullet();          // 玩家子弹（黄色 flare）
Texture2D enemyBullet();     // 敌方弹体（红色 circle）
Texture2D xpGem();           // 经验宝石（绿色 star）
Texture2D orbitalBlade();    // 环绕飞刃（银白 slash）

// ---- 统一绘制辅助：以 pos 为中心、worldSize 为目标宽度等比缩放，
//      rotationDeg 顺时针旋转（度），tint 染色（WHITE 为原色）----
void drawCentered(Texture2D tex, Vector2 pos, float worldSize,
                  float rotationDeg, Color tint);

// ---- 程序退出前释放全部已加载贴图 ----
void unloadAll();

}  // namespace assets
