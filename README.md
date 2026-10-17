# 竞技场幸存者（Arena Survivor）

俯视角竞技场生存射击游戏（Roguelite 随机成长 + 自动战斗），课程大作业项目，玩法参考《土豆兄弟》《Hall of Torment》。

## 目录结构

```
docs/    需求分析、系统分析、系统设计、开发计划等文档
assets/  音效、字体等资源（暂未使用）
src/     全部源代码（清单见下）
README.md 本文件
```

## src/ 文件清单（与 docs/03_系统设计.md §1.1 一致）

| 文件 | 归属模块 | 一句话职责 |
|---|---|---|
| main.cpp | 入口 | 创建 GameEngine 并进入主循环 |
| constants.h | 全局常量 | 数值基线集中定义（分模块命名空间） |
| game.h / game.cpp | 引擎核心 | GameEngine：状态机、固定时间步主循环 |
| player.h / player.cpp | 玩家 | 移动/受击/无敌帧/击退，持有武器与 Stats |
| stats.h / stats.cpp | 属性 | 属性字典，词条统一修改点 |
| weapon.h / weapon.cpp | 武器基座 | 抽象基类：索敌、射速计时、工厂注册 |
| weapon_pistol.h / .cpp | 武器 | 手枪（单发） |
| weapon_shotgun.h / .cpp | 武器 | 霰弹枪（扇形散射） |
| weapon_orbital.h / .cpp | 武器 | 环绕飞刃（持续旋转伤害） |
| projectile.h / projectile.cpp | 弹药 | 子弹与敌方弹体（含阵营标志） |
| enemy.h / enemy.cpp | 敌人基座 | 抽象基类：通用状态、受击框架 |
| enemy_crawler.h / .cpp | 敌人 | 爬行者（追踪 + 分离力） |
| enemy_spitter.h / .cpp | 敌人 | 喷吐者（保持距离射击） |
| enemy_brute.h / .cpp | 敌人 | 蛮牛（蓄力冲锋状态机） |
| wave_manager.h / .cpp | 波次 | 波次状态机与数量曲线 |
| enemy_spawner.h / .cpp | 生成 | 出生位置计算与敌人构造 |
| upgrade_system.h / .cpp | 升级 | 经验/等级/三选一/词条应用 |
| affix_library.h / .cpp | 词条 | 词条结构 + 词条池定义 |
| collision.h / collision.cpp | 碰撞 | 空间哈希网格 + 圆-圆检测 |
| ui.h / ui.cpp | 界面 | HUD、三选一面板、菜单与结算 |
| arena.h / arena.cpp | 场景 | 竞技场尺寸/围墙与边界钳制 |
| xp_orb.h / xp_orb.cpp | 掉落 | 经验点实体（运动、吸取） |

> 当前进度（T1）：上表文件均为骨架（类声明 + 空实现占位），仅 main.cpp / constants.h / game.h / game.cpp 已实现可运行内容。

## 环境要求

- Windows 10/11
- Visual Studio 2022（含"使用 C++ 的桌面开发"工作负载，自带 CMake 3.28）
- Git
- vcpkg（manifest 模式；本仓库默认假设克隆在工作区 .vcpkg/）
- 依赖库：raylib 4.5（由 vcpkg.json 的 builtin-baseline 锁定版本）

## 如何编译运行（PowerShell）

1. 获取 vcpkg（仅首次）：

   ```powershell
   git clone --depth 1 https://github.com/microsoft/vcpkg .vcpkg
   .\.vcpkg\bootstrap-vcpkg.bat
   ```

   > 注：vcpkg.json 使用浅克隆 HEAD 作为 builtin-baseline，并通过 overrides 将 raylib 锁定为 4.5.0（见 vcpkg.json），无需补齐完整历史。

2. 配置构建目录（首次会自动通过 vcpkg 下载并编译 raylib，需几分钟）：

   ```powershell
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$PWD\.vcpkg\scripts\buildsystems\vcpkg.cmake"
   ```

   vcpkg 装在其他位置的，把路径换成 <你的vcpkg目录>\scripts\buildsystems\vcpkg.cmake；cmake 不在 PATH 时可使用 VS 自带：

   ```powershell
   & "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B build -S . -DCMAKE_TOOLCHAIN_FILE="$PWD\.vcpkg\scripts\buildsystems\vcpkg.cmake"
   ```

3. 编译：

   ```powershell
   cmake --build build --config Release
   ```

4. 运行：

   ```powershell
   .\build\Release\arena_survivor.exe
   ```

## 操作说明（当前 T1 阶段）

- Esc：退出窗口
- 数字键 1~5：切换游戏状态（主菜单/进行/暂停/升级/结算），用于验收状态机空壳
- 窗口可任意缩放，画面保持 16:9 等比缩放（多余部分黑边）

## 开发进度

- [x] T1 项目骨架：环境、窗口、固定时间步主循环、五状态空壳
- [ ] T2 玩家移动与竞技场
- [ ] T3 敌人基础与怪物 AI
- [ ] T4 武器系统与自动攻击
- [ ] T5 碰撞检测与伤害
- [ ] T6 HUD 界面
- [ ] T7 升级与随机词条
- [ ] T8 波次系统与敌人多样化
- [ ] T9 数值平衡与内容扩展
- [ ] T10 完整游戏流程
- [ ] T11 打磨与优化（可选）
