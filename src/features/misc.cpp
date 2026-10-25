#include "misc.h"
#include "aimbot.h"
#include "core/globals.h"
#include "core/offsets.h"
#include "core/memory.h"
#include "game/sdk.h"
#include "ui/render.h"
#include "imgui.h"
#include <cstdio>

namespace misc {

static float s_fpsAccum = 0.f;
static int   s_fpsFrames = 0;

void Tick(float dt) {
    s_fpsAccum += dt; s_fpsFrames++;
    if (s_fpsAccum >= 0.5f) {
        g::fps = s_fpsFrames / s_fpsAccum;
        s_fpsAccum = 0.f; s_fpsFrames = 0;
    }

    if (cfg.misc.fovChanger) {
        uintptr_t cam = mem::ReadPtr(mem::g_base + off::CameraCandidate);
        if (cam) mem::Write<float>(cam + off::CameraFov, cfg.misc.gameFov);
    }
}

void Draw() {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    float scale = g::screenH / 1080.f;

    if (cfg.misc.watermark) {
        char buf[160];
        snprintf(buf, sizeof(buf), "VENICE // FIVEM  |  %.0f fps  |  %d players",
                 g::fps, g::playerCount);
        render::TextOutlined(dl, ImVec2(14.f, 12.f), buf, render::U32(cfg.misc.watermarkColor));
    }

    if (cfg.misc.playerCounter) {
        char buf[64];
        snprintf(buf, sizeof(buf), "PLAYERS: %d", g::playerCount);
        render::TextOutlined(dl, ImVec2(14.f, 34.f), buf, render::U32(palette::TextDim));
    }

    if (cfg.misc.crosshair) {
        float cx = g::screenW * 0.5f, cy = g::screenH * 0.5f;
        float s = cfg.misc.crosshairSize * scale;
        ImU32 c = render::U32(cfg.aim.color);
        dl->AddLine(ImVec2(cx - s, cy), ImVec2(cx - 2.f, cy), c, 1.5f);
        dl->AddLine(ImVec2(cx + 2.f, cy), ImVec2(cx + s, cy), c, 1.5f);
        dl->AddLine(ImVec2(cx, cy - s), ImVec2(cx, cy - 2.f), c, 1.5f);
        dl->AddLine(ImVec2(cx, cy + 2.f), ImVec2(cx, cy + s), c, 1.5f);
        if (cfg.misc.crosshairDot) dl->AddCircleFilled(ImVec2(cx, cy), 1.5f, c);
    }

    if (cfg.aim.enabled && cfg.aim.drawFov && !g::menuOpen) {
        float fovPx = cfg.aim.fov * (g::screenH / 1080.f) * 0.5f;
        ImVec2 c(g::screenW * 0.5f, g::screenH * 0.5f);
        RGBA col = cfg.aim.color;
        if (cfg.aim.fovFilled) {
            RGBA f = col; f.a *= 0.15f;
            dl->AddCircleFilled(c, fovPx, render::U32(f), 64);
        }
        dl->AddCircle(c, fovPx, render::U32(col), 64, 1.5f);
    }

    if (cfg.aim.enabled && cfg.aim.drawTarget && aimbot::IsTargeting()) {
        int ti = aimbot::CurrentTargetIndex();
        if (ti >= 0 && ti < (int)sdk::players.size()) {
            Player& t = sdk::players[ti];
            Vec2 s;
            if (sdk::WorldToScreen(t.pos, s)) {
                ImU32 c = render::U32(cfg.aim.color);
                dl->AddCircle(ImVec2(s.x, s.y), 6.f, c, 16, 1.5f);
                if (cfg.aim.drawLineToTarget) {
                    dl->AddLine(ImVec2(g::screenW * 0.5f, g::screenH * 0.5f),
                                ImVec2(s.x, s.y), c, 1.f);
                }
            }
        }
    }

    if (cfg.misc.boneDebug && sdk::localValid) {
        for (auto& p : sdk::players) {
            if (!p.valid) continue;
            for (int b = 0; b < 24; b++) {
                Vec3 w = sdk::GetBonePos(p.ped, b);
                Vec2 s;
                if ((w.x || w.y || w.z) && sdk::WorldToScreen(w, s)) {
                    dl->AddCircleFilled(ImVec2(s.x, s.y), 2.f, IM_COL32(255, 62, 150, 220));
                    if (b % 4 == 0) {
                        char t[8]; snprintf(t, sizeof(t), "%d", b);
                        dl->AddText(ImVec2(s.x + 4, s.y), IM_COL32(255, 255, 255, 180), t);
                    }
                }
            }
        }
    }

    if (cfg.veh.enabled) {
        uintptr_t replay = mem::ReadPtr(mem::g_base + off::ReplayInterface);
        if (replay) {
            uintptr_t iface = mem::ReadPtr(replay + off::ReplayVehicleIface);
            if (iface) {
                uintptr_t list = mem::ReadPtr(iface + off::VehicleIfaceList);
                int count = mem::Read<int>(iface + off::VehicleIfaceCount);
                count = count > 300 ? 300 : count;
                for (int i = 0; list && i < count; i++) {
                    uintptr_t veh = mem::ReadPtr(list + (size_t)i * 8);
                    if (!veh) continue;
                    Vec3 pos = mem::Read<Vec3>(veh + off::EntityPosition);
                    if (!(pos.x || pos.y || pos.z)) continue;
                    float d = Dist(pos, sdk::localPos);
                    if (d > cfg.veh.maxDistance) continue;
                    Vec2 s;
                    if (!sdk::WorldToScreen(pos, s)) continue;
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.0fm", d);
                    render::TextCentered(dl, s.x, s.y, buf, render::U32(cfg.veh.color));
                }
            }
        }
    }
}

} // namespace misc