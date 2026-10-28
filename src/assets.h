#pragma once
// ============================================================
// assets.h —— 全局素材集中管理（T12 美术资源替换；T13 子弹/宝石改程序化自绘）
// 角色贴图：Kenney.nl「Tiny Dungeon」CC0 像素 tile（16x16，详见
// assets/LICENSE_kenney_tiny_dungeon.txt），下载后已离线抠除 tile 深色底板，
// 加载时再按透明包围盒裁剪（内容居中）+ POINT 过滤保持像素锐利；
// 环绕飞刃沿用「Top-down Shooter」素材（assets/LICENSE_kenney_topdown_shooter.txt）。
// 子弹/敌弹/经验宝石自 T13 起由实体程序化绘制，不再占用贴图。
// 约定：所有贴图经本模块惰性加载并缓存，绘制方只查询不管理生命周期；
//       程序退出前统一调用 assets::unloadAll() 释放。
// ============================================================
#include "raylib.h"

namespace assets {

// ---- 贴图访问（惰性加载，首次调用 LoadTexture 并缓存）----
Texture2D player();          // 玩家（灰甲骑士，正面朝下，绘制时按朝向旋转）
Texture2D enemyCrawler();    // 爬行者（青绿史莱姆）
Texture2D enemySpitter();    // 喷吐者（红色小恶魔）
Texture2D enemyBrute();      // 蛮牛精英（橙棕食人魔）
Texture2D orbitalBlade();    // 环绕飞刃（银白 slash）

// ---- 统一绘制辅助：以 pos 为中心、worldSize 为目标宽度等比缩放，
//      rotationDeg 顺时针旋转（度），tint 染色（WHITE 为原色）----
void drawCentered(Texture2D tex, Vector2 pos, float worldSize,
                  float rotationDeg, Color tint);

// ---- 程序退出前释放全部已加载贴图 ----
void unloadAll();

}  // namespace assets
