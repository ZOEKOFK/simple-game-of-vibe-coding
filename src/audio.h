#pragma once
// ============================================================
// audio.h —— 程序生成音效（T11）
// 无外部资源依赖：InitAudioDevice 后实时合成 5 段短 WAV
//（正弦扫频/噪声包络 → 16bit 单声道 PCM → LoadWaveFromMemory → Sound）。
// 每类音效内置最小间隔限流，防止高频事件（命中/击杀）一帧叠爆。
// 音频设备不可用或合成失败时静默降级（ready()==false），游戏照常运行。
// ============================================================
#include "raylib.h"  // Sound

class SoundBank {
public:
    enum class Kind { Shoot, Hit, Kill, Upgrade, Death };

    bool init();       // 生成全部音效；失败静默返回 false
    void unload();     // 释放音效资源（CloseAudioDevice 之前调用）
    void play(Kind kind);  // 限流播放（未就绪 / 冷却中静默忽略）

    bool ready() const { return ready_; }

private:
    bool createSound(Kind kind);      // 合成该类型波形 → 生成 Sound
    Sound sounds_[5]{};
    double lastPlay_[5] = {0.0};      // 每类上次播放时刻（GetTime，秒）
    bool ready_ = false;
};
