// audio.cpp —— 程序生成音效实现（T11）
#include "audio.h"

#include <cmath>      // std::sin / std::exp / std::max
#include <vector>

namespace {

constexpr int kSampleRate = 22050;  // 采样率（Hz）；短音效内存可忽略

// 将 float 采样数组封成合法 16bit 单声道 WAV 字节流（供 LoadWaveFromMemory）
std::vector<unsigned char> buildWav(const std::vector<float>& samples) {
    const unsigned dataSize = static_cast<unsigned>(samples.size()) * 2u;
    const unsigned byteRate = static_cast<unsigned>(kSampleRate) * 2u;
    std::vector<unsigned char> wav;
    wav.reserve(44 + dataSize);
    const auto put16 = [&wav](unsigned v) {
        wav.push_back(static_cast<unsigned char>(v & 0xFF));
        wav.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
    };
    const auto put32 = [&wav](unsigned v) {
        wav.push_back(static_cast<unsigned char>(v & 0xFF));
        wav.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
        wav.push_back(static_cast<unsigned char>((v >> 16) & 0xFF));
        wav.push_back(static_cast<unsigned char>((v >> 24) & 0xFF));
    };
    const auto lit = [&wav](const char* s) {
        for (int i = 0; i < 4; ++i) wav.push_back(static_cast<unsigned char>(s[i]));
    };
    lit("RIFF"); put32(36 + dataSize); lit("WAVE");
    lit("fmt "); put32(16); put16(1); put16(1);       // PCM / 单声道
    put32(static_cast<unsigned>(kSampleRate)); put32(byteRate);
    put16(2); put16(16);                              // blockAlign / 位深
    lit("data"); put32(dataSize);
    for (float s : samples) {
        short v = static_cast<short>(s * 32767.0f);
        if (v < -32768) v = -32768;
        if (v > 32767) v = 32767;
        wav.push_back(static_cast<unsigned char>(v & 0xFF));
        wav.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
    }
    return wav;
}

// 线性扫频正弦 + 指数衰减（f0→f1；相位累计避免频率跳变咔哒声）
std::vector<float> sweep(double f0, double f1, float dur, float amp) {
    const int n = std::max(1, static_cast<int>(dur * kSampleRate));
    std::vector<float> out;
    out.reserve(n);
    double phase = 0.0;
    for (int i = 0; i < n; ++i) {
        const double t = static_cast<double>(i) / kSampleRate;
        const double f = f0 + (f1 - f0) * (t / dur);
        phase += 2.0 * 3.141592653589793 * f / kSampleRate;
        const double env = std::exp(-5.0 * t / dur);
        out.push_back(static_cast<float>(amp * std::sin(phase) * env));
    }
    return out;
}

// 白噪声爆发 + 指数衰减（命中/受击类短促声）
std::vector<float> noiseBurst(float dur, float amp) {
    const int n = std::max(1, static_cast<int>(dur * kSampleRate));
    std::vector<float> out;
    out.reserve(n);
    for (int i = 0; i < n; ++i) {
        const double t = static_cast<double>(i) / kSampleRate;
        const double env = std::exp(-8.0 * t / dur);
        const double rnd =
            (static_cast<double>(GetRandomValue(0, 32767)) / 32767.0) * 2.0 - 1.0;
        out.push_back(static_cast<float>(amp * rnd * env));
    }
    return out;
}

}  // namespace

bool SoundBank::createSound(Kind kind) {
    std::vector<float> samples;
    switch (kind) {
        case Kind::Shoot:   // 射击：880→240Hz 短促下滑（轻"啵"）
            samples = sweep(880.0, 240.0, 0.09f, 0.30f);
            break;
        case Kind::Hit:     // 命中：0.06s 噪声爆
            samples = noiseBurst(0.06f, 0.28f);
            break;
        case Kind::Kill:    // 击杀：420→80Hz 下坠"咚"
            samples = sweep(420.0, 80.0, 0.16f, 0.32f);
            break;
        case Kind::Upgrade: // 升级：C5→G5 双音上行琶音
        {
            const auto a = sweep(523.0, 523.0, 0.08f, 0.25f);
            const auto b = sweep(784.0, 784.0, 0.12f, 0.25f);
            samples.reserve(a.size() + b.size());
            samples.insert(samples.end(), a.begin(), a.end());
            samples.insert(samples.end(), b.begin(), b.end());
            break;
        }
        case Kind::Death:   // 死亡：300→55Hz 长下滑（低沉）
            samples = sweep(300.0, 55.0, 0.55f, 0.40f);
            break;
    }
    const auto wav = buildWav(samples);
    const Wave wave = LoadWaveFromMemory(".wav", wav.data(),
                                         static_cast<int>(wav.size()));
    if (wave.data == nullptr) return false;  // 合成/解析失败 → 该音效降级为空
    sounds_[static_cast<int>(kind)] = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return true;
}

bool SoundBank::init() {
    if (!IsAudioDeviceReady()) return false;
    bool ok = true;
    for (int i = 0; i < 5; ++i) {
        ok = createSound(static_cast<Kind>(i)) && ok;
    }
    ready_ = ok;
    return ready_;
}

void SoundBank::unload() {
    if (!ready_) return;
    for (int i = 0; i < 5; ++i) UnloadSound(sounds_[i]);
    ready_ = false;
}

void SoundBank::play(Kind kind) {
    if (!ready_) return;
    const int i = static_cast<int>(kind);
    const double now = GetTime();
    // 每类最小间隔（秒）：高频事件（命中/击杀/射击）限流，防止一帧叠爆
    constexpr double kMinInterval[5] = {0.045, 0.070, 0.060, 0.0, 0.0};
    if (now - lastPlay_[i] < kMinInterval[i]) return;
    lastPlay_[i] = now;
    PlaySound(sounds_[i]);
}
