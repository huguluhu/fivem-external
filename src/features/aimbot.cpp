#include "aimbot.h"
#include "core/globals.h"
#include "core/memory.h"
#include <cmath>

namespace aimbot {

static int   s_target = -1;
static float s_switchTimer = 0.f;
static float s_remX = 0.f, s_remY = 0.f;

static bool KeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

static bool ToggleState(int vk) {
    static bool held[256]{};
    static bool on[256]{};
    if (vk <= 0 || vk > 255) return false;
    bool now = KeyDown(vk);
    if (now && !held[vk]) on[vk] = !on[vk];
    held[vk] = now;
    return on[vk];
}

static bool AimActive() {
    if (cfg.aim.mode == 1) return ToggleState(cfg.aim.key);
    return KeyDown(cfg.aim.key);
}

static int HitboxBone(int hitbox) {
    switch (hitbox) {
    case 0: return 8;   // head
    case 1: return 7;   // neck
    case 2: return 5;   // chest
    case 3: return 1;   // pelvis
    default: return 8;
    }
}

static void MoveMouse(float dx, float dy) {
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dx = (LONG)dx;
    in.mi.dy = (LONG)dy;
    in.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &in, sizeof(INPUT));
}

bool IsTargeting() { return s_target >= 0 && s_target < (int)sdk::players.size(); }
int  CurrentTargetIndex() { return s_target; }

void Tick(float dt) {
    s_switchTimer -= dt;

    bool gameFocused = (GetForegroundWindow() == g::gameWindow);
    if (!cfg.aim.enabled || !gameFocused || !sdk::localValid) { s_target = -1; return; }

    bool active = AimActive();

    int best = -1;
    float bestMetric = 1e9f;

    Vec2 center(g::screenW * 0.5f, g::screenH * 0.5f);
    float fovPx = cfg.aim.fov * (g::screenH / 1080.f) * 0.5f;
    if (fovPx < 4.f) fovPx = 4.f;

    for (size_t i = 0; i < sdk::players.size(); i++) {
        Player& p = sdk::players[i];
        if (!p.valid) continue;
        if (cfg.aim.ignoreLocal && p.ped == sdk::localPed) continue;
        if (cfg.aim.ignoreDead && p.dead) continue;
        if (cfg.aim.visibleOnly && p.invisible) continue;
        if (p.distance > cfg.aim.maxDistance) continue;

        Vec3 aimPoint = sdk::GetBonePos(p.ped, HitboxBone(cfg.aim.hitbox));
        if (aimPoint.x == 0.f && aimPoint.y == 0.f && aimPoint.z == 0.f) aimPoint = p.pos;

        Vec2 screen;
        if (!sdk::WorldToScreen(aimPoint, screen)) continue;

        float dx = screen.x - center.x;
        float dy = screen.y - center.y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d > fovPx) continue;

        float metric;
        switch (cfg.aim.priority) {
        case 1:  metric = p.distance; break;
        case 2:  metric = p.health;   break;
        default: metric = d;          break;
        }
        if (metric < bestMetric) { bestMetric = metric; best = (int)i; }
    }

    if (best != s_target) {
        if (s_switchTimer > 0.f && s_target >= 0) { /* hold old target */ }
        else { s_target = best; s_switchTimer = cfg.aim.switchDelay; }
    }
    if (!active && !cfg.aim.lockTarget) s_target = best;

    if (s_target < 0 || s_target >= (int)sdk::players.size()) return;
    Player& t = sdk::players[s_target];
    if (!t.valid) { s_target = -1; return; }

    Vec3 aimPoint = sdk::GetBonePos(t.ped, HitboxBone(cfg.aim.hitbox));
    if (aimPoint.x == 0.f && aimPoint.y == 0.f && aimPoint.z == 0.f) aimPoint = t.pos;

    if (cfg.aim.prediction) {
        float flight = t.distance / 800.f;
        aimPoint = aimPoint + t.velocity * (flight * cfg.aim.predictionAmount * 30.f);
    }

    Vec2 screen;
    if (!sdk::WorldToScreen(aimPoint, screen)) return;

    float dx = screen.x - center.x;
    float dy = screen.y - center.y;

    if (cfg.aim.triggerbot) {
        float radius = cfg.aim.triggerRadius * (g::screenH / 1080.f);
        static float trigTimer = 0.f;
        if (std::fabs(dx) <= radius && std::fabs(dy) <= radius) {
            trigTimer += dt;
            if (trigTimer >= cfg.aim.triggerDelay) {
                INPUT in{};
                in.type = INPUT_MOUSE;
                in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
                SendInput(1, &in, sizeof(INPUT));
                trigTimer = 0.f;
            }
        } else trigTimer = 0.f;
    }

    if (!active) { s_remX = s_remY = 0.f; return; }

    float smooth = cfg.aim.smooth < 0.5f ? 0.5f : cfg.aim.smooth;
    if (cfg.aim.accelCurve) {
        float mag = std::sqrt(dx * dx + dy * dy);
        float factor = 1.f + (mag / 300.f);
        dx *= factor; dy *= factor;
    }

    float mx = Clampf(dx / smooth, -cfg.aim.maxStep, cfg.aim.maxStep);
    float my = Clampf(dy / smooth, -cfg.aim.maxStep, cfg.aim.maxStep);

    s_remX += mx; s_remY += my;
    LONG ix = (LONG)std::round(s_remX);
    LONG iy = (LONG)std::round(s_remY);
    if (ix == 0 && iy == 0) return;
    s_remX -= ix; s_remY -= iy;

    if (cfg.aim.aimMethod == 1) {
        POINT pt{}; GetCursorPos(&pt);
        SetCursorPos(pt.x + ix, pt.y + iy);
    } else {
        MoveMouse((float)ix, (float)iy);
    }
}

} // namespace aimbot