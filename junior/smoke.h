#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "global.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

struct SmokeParticle {
    float x, y;
    float vx, vy;
    float radius;
    float max_radius;
    float alpha;
    float max_alpha;
    float rotation;
    float rot_speed;
    float life;
    float max_life;
    float turb_phase[4];
    float turb_freq[4];
    float turb_amp[4];
    int   edge;
    bool  active;
};

struct SmokeConfig {
    static constexpr int MAX_PARTICLES    = 500;
    static constexpr int MAX_BURST        = 200;
    static constexpr float MENU_W         = 700.f;
    static constexpr float MENU_H         = 565.f;
    static constexpr float AMBIENT_RATE   = 18.f;
    static constexpr float BURST_LIFE_MIN = 1.2f;
    static constexpr float BURST_LIFE_MAX = 2.0f;
    static constexpr float AMBIENT_LIFE_MIN = 5.f;
    static constexpr float AMBIENT_LIFE_MAX = 8.f;
    static constexpr float TURB_STRENGTH  = 20.f;
    static constexpr float DRIFT_SPEED    = 8.f;
    static constexpr float EXPAND_RATE    = 0.32f;
};

class SmokeEmitter {
public:
    SmokeEmitter() : m_ambient_idx(0), m_burst_idx(0), m_ambient_timer(0.f), m_rand_seed(12345) {}

    void Init() {
        memset(m_ambient, 0, sizeof(m_ambient));
        memset(m_burst, 0, sizeof(m_burst));
        for (auto& p : m_ambient) { p.active = false; p.life = 999.f; }
        for (auto& p : m_burst)   { p.active = false; p.life = 999.f; }
    }

    void Update(float dt, float time, bool burst) {
        float speed = global::menu::particle_speed;
        m_ambient_timer += dt;
        float spawn_interval = 1.f / SmokeConfig::AMBIENT_RATE;

        while (m_ambient_timer >= spawn_interval) {
            m_ambient_timer -= spawn_interval;
            SpawnAmbient();
        }

        if (burst) {
            SpawnBurst();
        }

        for (auto& p : m_ambient) UpdateParticle(p, dt * speed, time);
        for (auto& p : m_burst)   UpdateParticle(p, dt * speed, time);
    }

    SmokeParticle* GetAmbient()  { return m_ambient; }
    SmokeParticle* GetBurst()    { return m_burst; }
    int AmbientCount() const     { return SmokeConfig::MAX_PARTICLES; }
    int BurstCount() const       { return SmokeConfig::MAX_BURST; }

private:
    float RandF(float lo, float hi) {
        m_rand_seed = m_rand_seed * 1103515245 + 12345;
        float t = (float)((m_rand_seed >> 16) & 0x7FFF) / 32767.f;
        return lo + t * (hi - lo);
    }

    void SetupTurbulence(SmokeParticle& p, float strength) {
        p.turb_freq[0] = RandF(0.35f, 1.1f);
        p.turb_freq[1] = RandF(0.25f, 0.85f);
        p.turb_freq[2] = RandF(0.55f, 1.6f);
        p.turb_freq[3] = RandF(0.4f, 1.05f);
        p.turb_amp[0]  = strength * RandF(0.6f, 1.4f);
        p.turb_amp[1]  = strength * RandF(0.5f, 1.2f);
        p.turb_amp[2]  = strength * RandF(0.3f, 0.8f);
        p.turb_amp[3]  = strength * RandF(0.4f, 1.0f);
    }

    void SpawnAmbient() {
        SmokeParticle& p = m_ambient[m_ambient_idx];
        m_ambient_idx = (m_ambient_idx + 1) % SmokeConfig::MAX_PARTICLES;

        float W = SmokeConfig::MENU_W;
        float H = SmokeConfig::MENU_H;
        p.edge = rand() % 6;

        float cx, cy, inward_angle;
        GetEdgePos(p.edge, cx, cy);
        inward_angle = GetInwardAngle(p.edge);

        p.x = cx + RandF(-35.f, 35.f);
        p.y = cy + RandF(-35.f, 35.f);

        float drift = SmokeConfig::DRIFT_SPEED * RandF(0.3f, 1.0f);
        p.vx = cosf(inward_angle) * drift + RandF(-4.f, 4.f);
        p.vy = sinf(inward_angle) * drift + RandF(-4.f, 4.f);

        p.max_radius  = 80.f + RandF(0.f, 180.f);
        p.radius      = p.max_radius * 0.15f;
        p.max_alpha   = 0.14f + RandF(0.f, 0.10f);
        p.alpha       = 0.f;
        p.rotation    = RandF(0.f, 6.28f);
        p.rot_speed   = RandF(-0.35f, 0.35f);
        p.life        = 0.f;
        p.max_life    = RandF(SmokeConfig::AMBIENT_LIFE_MIN, SmokeConfig::AMBIENT_LIFE_MAX);
        p.active      = true;

        SetupTurbulence(p, SmokeConfig::TURB_STRENGTH);
    }

    void SpawnBurst() {
        for (int i = 0; i < SmokeConfig::MAX_BURST; i++) {
            SmokeParticle& p = m_burst[i];

            int edge = i % 6;
            float cx, cy;
            GetEdgePos(edge, cx, cy);
            p.edge = edge;
            p.x = cx + RandF(-30.f, 30.f);
            p.y = cy + RandF(-30.f, 30.f);

            float inward = GetInwardAngle(edge);
            float spread  = RandF(-0.8f, 0.8f);
            float speed   = SmokeConfig::DRIFT_SPEED * RandF(1.8f, 3.5f);
            p.vx = cosf(inward + spread) * speed;
            p.vy = sinf(inward + spread) * speed;

            p.max_radius  = 60.f + RandF(0.f, 160.f);
            p.radius      = p.max_radius * 0.12f;
            p.max_alpha   = 0.22f + RandF(0.f, 0.12f);
            p.alpha       = 0.f;
            p.rotation    = RandF(0.f, 6.28f);
            p.rot_speed   = RandF(-0.6f, 0.6f);
            p.life        = 0.f;
            p.max_life    = RandF(SmokeConfig::BURST_LIFE_MIN, SmokeConfig::BURST_LIFE_MAX);
            p.active      = true;

            SetupTurbulence(p, SmokeConfig::TURB_STRENGTH * 1.6f);
        }
    }

    void UpdateParticle(SmokeParticle& p, float dt, float time) {
        if (!p.active) return;
        p.life += dt;
        if (p.life >= p.max_life) { p.active = false; return; }

        float t = p.life / p.max_life;

        float turb_x = 0.f, turb_y = 0.f;
        for (int k = 0; k < 4; k++) {
            float phase = p.turb_phase[k] + time * p.turb_freq[k];
            if (k < 2) turb_x += sinf(phase) * p.turb_amp[k];
            else       turb_y += cosf(phase) * p.turb_amp[k];
        }
        p.x += (p.vx + turb_x) * dt;
        p.y += (p.vy + turb_y) * dt;

        p.vx *= 0.998f;
        p.vy *= 0.998f;

        p.radius = p.max_radius * (0.15f + SmokeConfig::EXPAND_RATE * t * 2.4f);
        if (p.radius > p.max_radius) p.radius = p.max_radius;

        p.rotation += p.rot_speed * dt;

        float ease_in  = t < 0.15f ? (t / 0.15f) : 1.f;
        float ease_out = 1.f - (t * t * t);
        p.alpha = p.max_alpha * ease_in * ease_out;
    }

    void GetEdgePos(int edge, float& cx, float& cy) {
        float W = SmokeConfig::MENU_W;
        float H = SmokeConfig::MENU_H;
        switch (edge) {
        case 0: cx = 0.f;           cy = RandF(0.f, H);          break;
        case 1: cx = W;             cy = RandF(0.f, H);          break;
        case 2: cx = RandF(0.f, W); cy = 0.f;                    break;
        case 3: cx = RandF(0.f, W); cy = H;                      break;
        case 4: cx = 0.f;           cy = 0.f;                    break;
        case 5: cx = W;             cy = H;                      break;
        }
    }

    float GetInwardAngle(int edge) {
        float W = SmokeConfig::MENU_W;
        float H = SmokeConfig::MENU_H;
        float tx = W * 0.5f;
        float ty = H * 0.5f;
        float cx, cy;
        GetEdgePos(edge, cx, cy);
        return atan2f(ty - cy, tx - cx);
    }

    SmokeParticle m_ambient[SmokeConfig::MAX_PARTICLES];
    SmokeParticle m_burst[SmokeConfig::MAX_BURST];
    int  m_ambient_idx;
    int  m_burst_idx;
    float m_ambient_timer;
    unsigned int m_rand_seed;
};

class SmokeRenderer {
public:
    static void RenderSoftCircle(ImDrawList* draw, ImVec2 center, float radius, ImU32 color) {
        draw->AddCircleFilled(center, radius, color, 48);
    }

    static void Render(ImDrawList* draw, ImVec2 menu_pos, SmokeParticle* particles, int count, float time) {
        for (int i = 0; i < count; i++) {
            const SmokeParticle& p = particles[i];
            if (!p.active || p.alpha < 0.001f) continue;

            float px = menu_pos.x + p.x;
            float py = menu_pos.y + p.y;

            unsigned int base_a = (unsigned int)(p.alpha * 255.f);
            if (base_a < 1) continue;

            int rings = 7;
            for (int ring = rings; ring >= 0; ring--) {
                float ring_t = (float)ring / (float)rings;
                float rr = p.radius * (0.1f + ring_t * 0.9f);
                float ra = p.alpha * (1.f - ring_t * 0.8f) * 0.88f;
                unsigned int a = (unsigned int)(ra * 255.f);
                if (a < 1) continue;

                float offset_x = cosf(p.rotation + ring_t * 2.5f) * ring_t * 7.f;
                float offset_y = sinf(p.rotation + ring_t * 2.5f) * ring_t * 7.f;

                int cr, cg, cb;
                cr = (int)(120.f + ring_t * (-50.f)); cg = (int)(80.f + ring_t * (-50.f)); cb = (int)(200.f + ring_t * 55.f);
                if (cr < 0) cr = 0; if (cr > 255) cr = 255;
                if (cg < 0) cg = 0; if (cg > 255) cg = 255;
                if (cb < 0) cb = 0; if (cb > 255) cb = 255;

                ImU32 col = IM_COL32(cr, cg, cb, a);
                RenderSoftCircle(draw, ImVec2(px + offset_x, py + offset_y), rr, col);
            }

            float core_r = p.radius * 0.22f;
            unsigned int core_a = (unsigned int)(p.alpha * 0.3f * 255.f);
            if (core_a > 0) {
                int ccr, ccg, ccb;
                ccr = 220; ccg = 200; ccb = 255;
                ImU32 core_col = IM_COL32(ccr, ccg, ccb, core_a);
                RenderSoftCircle(draw, ImVec2(px, py), core_r, core_col);
            }
        }
    }
};

class SmokeSystem {
public:
    SmokeSystem() : m_init(false) {}

    void Init() {
        if (m_init) return;
        m_emitter.Init();
        m_init = true;
    }

    void Update(float dt, bool burst_requested) {
        if (!m_init) Init();
        float time = (float)ImGui::GetTime();
        m_emitter.Update(dt, time, burst_requested);
    }

    void Render(ImDrawList* draw, ImVec2 menu_pos) {
        float time = (float)ImGui::GetTime();
        SmokeRenderer::Render(draw, menu_pos, m_emitter.GetAmbient(), m_emitter.AmbientCount(), time);
        SmokeRenderer::Render(draw, menu_pos, m_emitter.GetBurst(),   m_emitter.BurstCount(),   time);
    }

    SmokeEmitter& Emitter() { return m_emitter; }

private:
    SmokeEmitter m_emitter;
    bool m_init;
};

inline SmokeSystem g_smokeSystem;
