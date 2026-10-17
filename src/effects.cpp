// effects.cpp —— 打击感表现系统实现（T11）
#include "effects.h"

#include <cmath>   // std::cos / std::sin

#include "constants.h"  // fx 常量（击杀飘字存活时长）

// ============================================================
// 粒子与飘字：世界坐标实体，寿命递减，绘制时按剩余占比做大小/透明度衰减
// ============================================================

void Effects::update(float dt) {
    for (auto it = particles_.begin(); it != particles_.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = particles_.erase(it);
        } else {
            it->pos.x += it->vel.x * dt;
            it->pos.y += it->vel.y * dt;
            ++it;
        }
    }
    for (auto it = texts_.begin(); it != texts_.end();) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = texts_.erase(it);
        } else {
            it->pos.y -= 32.0f * dt;  // 击杀飘字缓慢上升
            ++it;
        }
    }
}

void Effects::draw() const {
    for (const auto& p : particles_) {
        const float k = p.life / p.maxLife;  // 1→0 衰减
        const Color c{p.color.r, p.color.g, p.color.b,
                      static_cast<unsigned char>(p.color.a * k)};
        DrawCircleV(p.pos, p.size * (0.4f + 0.6f * k), c);
    }
    for (const auto& t : texts_) {
        const unsigned char a =
            static_cast<unsigned char>(255.0f * (t.life / t.maxLife));
        const Color c{t.color.r, t.color.g, t.color.b, a};
        const int w = MeasureText(t.text.c_str(), 18);
        DrawText(t.text.c_str(), static_cast<int>(t.pos.x - w * 0.5f),
                 static_cast<int>(t.pos.y), 18, c);
    }
}

void Effects::spawnBurst(const Vector2& pos, int count, Color color,
                         float speed, float life) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;
        // 均匀方向 + 速度/寿命/大小的随机抖动（GetRandomValue 上限 32767 内）
        const float ang = static_cast<float>(GetRandomValue(0, 6283)) / 1000.0f;  // 0~2π
        const float sp  = speed * (0.4f + 0.6f * GetRandomValue(0, 1000) / 1000.0f);
        p.vel = Vector2{std::cos(ang) * sp, std::sin(ang) * sp};
        p.life = p.maxLife = life * (0.6f + 0.4f * GetRandomValue(0, 1000) / 1000.0f);
        p.size = 2.5f + 2.5f * GetRandomValue(0, 1000) / 1000.0f;
        p.color = color;
        particles_.push_back(p);
    }
}

void Effects::spawnKillText(const Vector2& pos, const std::string& text,
                            Color color) {
    KillText t;
    t.pos = pos;
    t.text = text;
    t.life = t.maxLife = constants::fx::kKillTextLife;
    t.color = color;
    texts_.push_back(t);
}

void Effects::addShake(float amplitude, float duration) {
    shakeAmp_ = amplitude;
    shakeDur_ = duration;
    shakeStart_ = GetTime();
}

Vector2 Effects::shakeOffset() const {
    if (shakeDur_ <= 0.0f || shakeAmp_ <= 0.0f) return Vector2{};
    const double now = GetTime();
    const double elapsed = now - shakeStart_;
    if (elapsed >= shakeDur_) return Vector2{};
    const float k = 1.0f - static_cast<float>(elapsed / shakeDur_);  // 1→0 线性衰减
    const float a = shakeAmp_ * k;
    return Vector2{static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f * a,
                   static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f * a};
}

void Effects::clear() {
    particles_.clear();
    texts_.clear();
    shakeAmp_ = 0.0f;
    shakeDur_ = 0.0f;
}
