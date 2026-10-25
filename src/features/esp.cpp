#include "esp.h"
#include "core/globals.h"
#include "core/offsets.h"
#include "core/memory.h"
#include "game/sdk.h"
#include "ui/render.h"
#include "imgui.h"
#include <cstdio>

namespace esp {

static const int kSkeleton[][2] = {
    {8,7},{7,6},{6,5},{5,4},{4,3},{3,1},
    {6,9},{9,10},{10,11},{11,12},
    {6,13},{13,14},{14,15},{15,16},
    {1,17},{17,18},{18,19},
    {1,20},{20,21},{21,22}
};

struct Box { ImVec2 min, max; bool ok = false; };

static Box BuildBox(uintptr_t ped, const Vec3& fallback) {
    Box b;
    Vec2 head, lf, rf;
    Vec3 headW = sdk::GetBonePos(ped, 8);
    Vec3 lfW   = sdk::GetBonePos(ped, 19);
    Vec3 rfW   = sdk::GetBonePos(ped, 22);

    bool hasBones = sdk::WorldToScreen(headW, head) &&
                    sdk::WorldToScreen(lfW, lf) &&
                    sdk::WorldToScreen(rfW, rf);
    if (!hasBones) {
        Vec2 feet;
        if (!sdk::WorldToScreen(fallback, feet)) return b;
        head = Vec2(feet.x, feet.y - 90.f);
        lf = rf = feet;
    }

    float topY    = head.y;
    float bottomY = (lf.y > rf.y) ? lf.y : rf.y;
    float cx      = (head.x + lf.x + rf.x) / 3.f;
    float h       = bottomY - topY;
    if (h < 4.f) return b;

    float w = h * 0.42f;
    b.min = ImVec2(cx - w * 0.5f, topY - h * 0.08f);
    b.max = ImVec2(cx + w * 0.5f, bottomY);
    b.ok = true;
    return b;
}

void Draw() {
    if (!cfg.esp.enabled || !sdk::localValid) return;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float scale = g::screenH / 1080.f;

    for (auto& p : sdk::players) {
        if (!p.valid) continue;
        if (cfg.esp.hideLocal && p.ped == sdk::localPed) continue;
        if (p.distance > cfg.esp.maxDistance) continue;

        RGBA col = p.invisible ? cfg.esp.invisibleColor : cfg.esp.visibleColor;
        ImU32 c = render::U32(col);

        Vec2 feetScreen;
        if (!sdk::WorldToScreen(p.pos, feetScreen)) continue;

        if (cfg.esp.snaplines) {
            float sy = cfg.esp.snapOrigin == 0 ? 0.f
                     : cfg.esp.snapOrigin == 1 ? g::screenH * 0.5f
                     : (float)g::screenH;
            dl->AddLine(ImVec2((float)g::screenW * 0.5f, sy),
                        ImVec2(feetScreen.x, feetScreen.y), c, 1.f);
        }

        Box box = BuildBox(p.ped, p.pos);
        if (box.ok) {
            if (cfg.esp.boxFill) {
                RGBA f = col; f.a = cfg.esp.boxFillAlpha;
                dl->AddRectFilled(box.min, box.max, render::U32(f));
            }
            if (cfg.esp.boxes) {
                if (cfg.esp.boxStyle == 0)
                    dl->AddRect(box.min, box.max, c, 0.f, 0, cfg.esp.boxThickness);
                else
                    render::CornerBox(dl, box.min, box.max, c, cfg.esp.boxThickness,
                                      (box.max.y - box.min.y) * 0.22f);
            }

            float barW = 3.f * scale;
            if (cfg.esp.health) {
                float pct = p.health / (p.maxHealth > 0.f ? p.maxHealth : 200.f);
                render::HealthBar(dl, ImVec2(box.min.x - barW - 3.f, box.min.y),
                                  ImVec2(box.min.x - 3.f, box.max.y), pct,
                                  render::U32(cfg.esp.healthColor));
            }
            if (cfg.esp.armor) {
                float pct = Clampf(p.armor / 100.f, 0.f, 1.f);
                render::HealthBar(dl, ImVec2(box.max.x + 3.f, box.min.y),
                                  ImVec2(box.max.x + 3.f + barW, box.max.y), pct,
                                  render::U32(cfg.esp.armorColor));
            }

            if (cfg.esp.headDot) {
                Vec2 headScreen;
                Vec3 headW = sdk::GetBonePos(p.ped, 8);
                if (sdk::WorldToScreen(headW, headScreen))
                    dl->AddCircleFilled(ImVec2(headScreen.x, headScreen.y), 3.f * scale, c);
            }

            float cx = (box.min.x + box.max.x) * 0.5f;
            float ty = box.min.y;
            bool outline = cfg.esp.textOutline;

            if (cfg.esp.names && !p.name.empty()) {
                ty -= 16.f * scale;
                render::TextCentered(dl, cx, ty, p.name.c_str(),
                                     render::U32(cfg.esp.nameColor), outline);
            }
            if (cfg.esp.distance) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%.0fm", p.distance);
                render::TextCentered(dl, cx, box.max.y + 3.f, buf,
                                     render::U32(cfg.esp.distColor), outline);
            }
            if (cfg.esp.weapons && p.weaponHash) {
                std::string w = sdk::WeaponName(p.weaponHash);
                render::TextCentered(dl, cx, box.max.y + 19.f, w.c_str(),
                                     render::U32(cfg.esp.distColor), outline);
            }
        }

        if (cfg.esp.skeleton) {
            Vec2 pts[23];
            bool ok[23]{};
            for (int i = 0; i < 23; i++) {
                Vec3 w = sdk::GetBonePos(p.ped, i);
                ok[i] = (w.x != 0.f || w.y != 0.f || w.z != 0.f) &&
                        sdk::WorldToScreen(w, pts[i]);
            }
            ImU32 sc = render::U32(cfg.esp.skelColor);
            for (auto& pair : kSkeleton) {
                if (ok[pair[0]] && ok[pair[1]])
                    dl->AddLine(ImVec2(pts[pair[0]].x, pts[pair[0]].y),
                                ImVec2(pts[pair[1]].x, pts[pair[1]].y),
                                sc, cfg.esp.skelThickness);
            }
        }
    }
}

} // namespace esp