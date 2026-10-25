#include "menu.h"
#include "core/globals.h"
#include "core/memory.h"
#include "game/sdk.h"
#include "config/config.h"
#include "ui/notify.h"
#include "ui/render.h"
#include "scripting/lua_engine.h"
#include "theme.h"
#include "imgui.h"
#include <cstdio>
#include <cstring>

namespace menu {

static int   s_tab = 0;
static int*  s_binding = nullptr;
static float s_bindDelay = 0.f;
static char  s_cfgName[64] = "default";
static char  s_luaInput[512] = "";
static char  s_path[260] = "";

static const char* kTabs[] = { "AIMBOT", "ESP", "VISUALS", "MISC", "LUA", "SETTINGS" };

static const char* KeyName(int vk) {
    static char buf[32];
    switch (vk) {
    case 0: return "none";
    case VK_LBUTTON: return "Mouse1";
    case VK_RBUTTON: return "Mouse2";
    case VK_MBUTTON: return "Mouse3";
    case VK_XBUTTON1: return "Mouse4";
    case VK_XBUTTON2: return "Mouse5";
    case VK_LSHIFT: return "LShift";
    case VK_RSHIFT: return "RShift";
    case VK_LCONTROL: return "LCtrl";
    case VK_RCONTROL: return "RCtrl";
    case VK_LMENU: return "LAlt";
    case VK_RMENU: return "RAlt";
    case VK_SPACE: return "Space";
    case VK_TAB: return "Tab";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HOME: return "Home";
    case VK_END: return "End";
    case VK_PRIOR: return "PgUp";
    case VK_NEXT: return "PgDn";
    case VK_CAPITAL: return "CapsLock";
    default: break;
    }
    if (vk >= VK_F1 && vk <= VK_F24) { sprintf_s(buf, "F%d", vk - VK_F1 + 1); return buf; }
    if (vk >= '0' && vk <= '9') { sprintf_s(buf, "%c", (char)vk); return buf; }
    if (vk >= 'A' && vk <= 'Z') { sprintf_s(buf, "%c", (char)vk); return buf; }
    sprintf_s(buf, "0x%02X", vk);
    return buf;
}

bool WantsKeybind() { return s_binding != nullptr; }

static void KeybindRow(const char* label, int& key) {
    ImGui::PushID(label);
    bool listening = (s_binding == &key);
    char text[48];
    if (listening) strcpy_s(text, "...");
    else sprintf_s(text, "%s", KeyName(key));

    if (listening) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, .24f, .59f, .45f));
    if (ImGui::Button(text, ImVec2(96, 0)) && !listening) {
        s_binding = &key;
        s_bindDelay = 0.15f;
    }
    if (listening) ImGui::PopStyleColor();

    if (listening && s_bindDelay <= 0.f) {
        for (int k = 1; k < 255; k++) {
            if (k == VK_LBUTTON || k == VK_RBUTTON) continue;
            if (GetAsyncKeyState(k) & 0x8000) { key = k; s_binding = nullptr; break; }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) s_binding = nullptr;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    ImGui::PopID();
}

static void Section(const char* title) {
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, .24f, .59f, 1.f));
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
    ImGui::Separator();
    ImGui::Spacing();
}

static void ColorRow(const char* label, RGBA& c) {
    ImGui::ColorEdit4(label, &c.r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
}

static void DrawAimbot() {
    ImGui::Checkbox("Enable Aimbot", &cfg.aim.enabled);
    ImGui::SameLine(280); KeybindRow("Aim Key", cfg.aim.key);

    const char* modes[] = { "Hold", "Toggle" };
    ImGui::Combo("Mode", &cfg.aim.mode, modes, 2);

    Section("Targeting");
    ImGui::SliderFloat("FOV", &cfg.aim.fov, 1.f, 180.f, "%.0f");
    ImGui::Checkbox("Draw FOV Circle", &cfg.aim.drawFov);
    ImGui::SameLine(); ImGui::Checkbox("Filled", &cfg.aim.fovFilled);
    ColorRow("Aimbot Color", cfg.aim.color);

    const char* hitboxes[] = { "Head", "Neck", "Chest", "Pelvis", "Nearest", "Random" };
    ImGui::Combo("Hitbox", &cfg.aim.hitbox, hitboxes, 6);

    const char* prios[] = { "Closest to Crosshair", "Closest Distance", "Lowest Health" };
    ImGui::Combo("Priority", &cfg.aim.priority, prios, 3);

    ImGui::SliderFloat("Max Distance", &cfg.aim.maxDistance, 10.f, 1000.f, "%.0fm");
    ImGui::Checkbox("Lock Target", &cfg.aim.lockTarget);
    ImGui::SameLine(); ImGui::Checkbox("Visible Only", &cfg.aim.visibleOnly);
    ImGui::Checkbox("Ignore Dead", &cfg.aim.ignoreDead);
    ImGui::SameLine(); ImGui::Checkbox("Ignore NPCs", &cfg.aim.ignoreNpc);
    ImGui::Checkbox("Ignore Local", &cfg.aim.ignoreLocal);
    ImGui::SliderFloat("Switch Delay", &cfg.aim.switchDelay, 0.f, 1.f, "%.2fs");

    Section("Smoothing");
    ImGui::SliderFloat("Smooth", &cfg.aim.smooth, 0.5f, 20.f, "%.1f");
    ImGui::Checkbox("Acceleration Curve", &cfg.aim.accelCurve);
    ImGui::SliderFloat("Max Step", &cfg.aim.maxStep, 10.f, 2000.f, "%.0f");
    const char* methods[] = { "Mouse Move (SendInput)", "Set Cursor Position" };
    ImGui::Combo("Method", &cfg.aim.aimMethod, methods, 2);

    Section("Prediction");
    ImGui::Checkbox("Enable Prediction", &cfg.aim.prediction);
    ImGui::SliderFloat("Prediction Amount", &cfg.aim.predictionAmount, 0.f, 3.f, "%.2f");

    Section("Triggerbot");
    ImGui::Checkbox("Enable Triggerbot", &cfg.aim.triggerbot);
    ImGui::SliderFloat("Trigger Radius", &cfg.aim.triggerRadius, 0.5f, 20.f, "%.1f");
    ImGui::SliderFloat("Trigger Delay", &cfg.aim.triggerDelay, 0.f, 0.3f, "%.3fs");

    Section("Indicators");
    ImGui::Checkbox("Target Marker", &cfg.aim.drawTarget);
    ImGui::SameLine(); ImGui::Checkbox("Line to Target", &cfg.aim.drawLineToTarget);
}

static void DrawEsp() {
    ImGui::Checkbox("Enable ESP", &cfg.esp.enabled);
    ImGui::SameLine(); ImGui::Checkbox("Hide Local Player", &cfg.esp.hideLocal);

    Section("Box");
    ImGui::Checkbox("Boxes", &cfg.esp.boxes);
    const char* styles[] = { "Full", "Cornered" };
    ImGui::Combo("Style", &cfg.esp.boxStyle, styles, 2);
    ImGui::SliderFloat("Thickness", &cfg.esp.boxThickness, 0.5f, 6.f, "%.1f");
    ImGui::Checkbox("Fill", &cfg.esp.boxFill);
    ImGui::SliderFloat("Fill Alpha", &cfg.esp.boxFillAlpha, 0.f, 1.f, "%.2f");

    Section("Skeleton");
    ImGui::Checkbox("Skeleton", &cfg.esp.skeleton);
    ImGui::SliderFloat("Bone Thickness", &cfg.esp.skelThickness, 0.5f, 6.f, "%.1f");

    Section("Info");
    ImGui::Checkbox("Names", &cfg.esp.names);
    ImGui::SameLine(); ImGui::Checkbox("Distance", &cfg.esp.distance);
    ImGui::Checkbox("Health Bar", &cfg.esp.health);
    ImGui::SameLine(); ImGui::Checkbox("Armor Bar", &cfg.esp.armor);
    ImGui::Checkbox("Weapon", &cfg.esp.weapons);
    ImGui::SameLine(); ImGui::Checkbox("Head Dot", &cfg.esp.headDot);
    ImGui::Checkbox("Text Outline", &cfg.esp.textOutline);

    Section("Snaplines");
    ImGui::Checkbox("Snaplines", &cfg.esp.snaplines);
    const char* origins[] = { "Top", "Center", "Bottom" };
    ImGui::Combo("Origin", &cfg.esp.snapOrigin, origins, 3);

    Section("Colors");
    ColorRow("Visible", cfg.esp.visibleColor);
    ColorRow("Invisible", cfg.esp.invisibleColor);
    ColorRow("Skeleton", cfg.esp.skelColor);
    ColorRow("Name", cfg.esp.nameColor);
    ColorRow("Health", cfg.esp.healthColor);
    ColorRow("Armor", cfg.esp.armorColor);
    ColorRow("Distance", cfg.esp.distColor);

    Section("Range");
    ImGui::SliderFloat("Max ESP Distance", &cfg.esp.maxDistance, 10.f, 1500.f, "%.0fm");

    Section("Vehicles");
    ImGui::Checkbox("Vehicle ESP", &cfg.veh.enabled);
    ImGui::Checkbox("Vehicle Box", &cfg.veh.box);
    ImGui::Checkbox("Vehicle Distance", &cfg.veh.distance);
    ImGui::SliderFloat("Vehicle Range", &cfg.veh.maxDistance, 10.f, 1000.f, "%.0fm");
    ColorRow("Vehicle Color", cfg.veh.color);
}

static void DrawVisuals() {
    Section("Overlay");
    ImGui::Checkbox("Watermark", &cfg.misc.watermark);
    ColorRow("Watermark Color", cfg.misc.watermarkColor);
    ImGui::Checkbox("FPS Counter", &cfg.misc.fpsCounter);
    ImGui::SameLine(); ImGui::Checkbox("Player Counter", &cfg.misc.playerCounter);

    Section("Crosshair");
    ImGui::Checkbox("Custom Crosshair", &cfg.misc.crosshair);
    ImGui::SliderFloat("Size", &cfg.misc.crosshairSize, 2.f, 40.f, "%.0f");
    ImGui::Checkbox("Center Dot", &cfg.misc.crosshairDot);

    Section("Camera");
    ImGui::Checkbox("Game FOV Changer", &cfg.misc.fovChanger);
    ImGui::SliderFloat("FOV", &cfg.misc.gameFov, 40.f, 130.f, "%.0f");
    ImGui::Checkbox("Speedometer", &cfg.misc.speedometer);
}

static void DrawMisc() {
    Section("Menu");
    KeybindRow("Menu Key", cfg.misc.menuKey);

    Section("Debug / Offsets");
    ImGui::Text("Bone Layout Mode");
    ImGui::RadioButton("fragInst + 0x1A0", &g::boneMode, 0); ImGui::SameLine();
    ImGui::RadioButton("fragInst + 0x20", &g::boneMode, 1);  ImGui::SameLine();
    ImGui::RadioButton("fragInst direct", &g::boneMode, 2);
    ImGui::SliderInt("Bone Stride", &g::boneStride, 0x10, 0x100, "0x%X");
    ImGui::SliderInt("Bone Position Offset", &g::bonePosOff, 0x0, 0x60, "0x%X");
    ImGui::Checkbox("Show Bone Debug", &cfg.misc.boneDebug);

    const char* srcNames[] = { "direct global", "camera + 0x24C", "camera + 0x0", "unresolved" };
    int si = g::viewMatrixSrc < 0 ? 3 : g::viewMatrixSrc;
    ImGui::Text("View Matrix Source: %s", srcNames[si]);

    Section("Status");
    ImGui::Text("Process: %s", g::attached ? "attached" : "waiting...");
    ImGui::Text("Module Base: 0x%llX", (unsigned long long)mem::g_base);
    ImGui::Text("Local Ped: 0x%llX", (unsigned long long)sdk::localPed);
    ImGui::Text("Players Cached: %d", (int)sdk::players.size());
    ImGui::Text("Screen: %dx%d", g::screenW, g::screenH);
    ImGui::Text("FPS: %.1f", g::fps);
}

static void DrawLua() {
    Section("Console");

    ImGui::BeginChild("luallog", ImVec2(0, 240), true, ImGuiWindowFlags_HorizontalScrollbar);
    auto lines = lua::Snapshot();
    for (auto& l : lines) {
        ImVec4 col = ImVec4(0.85f, 0.85f, 0.88f, 1.f);
        if (!l.empty() && l[0] == '>') col = ImVec4(1.f, .35f, .65f, 1.f);
        if (l.find("[error]") != std::string::npos) col = ImVec4(1.f, .35f, .35f, 1.f);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextUnformatted(l.c_str());
        ImGui::PopStyleColor();
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.f) ImGui::SetScrollHereY(1.f);
    ImGui::EndChild();

    ImGui::PushFont(theme::fontMono);
    ImGui::SetNextItemWidth(-120.f);
    bool submit = ImGui::InputText("##lua", s_luaInput, sizeof(s_luaInput),
                                   ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::PopFont();
    ImGui::SameLine();
    if (ImGui::Button("Run", ImVec2(52, 0)) || submit) {
        if (s_luaInput[0]) {
            lua::RunString(std::string(s_luaInput));
            s_luaInput[0] = 0;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(56, 0))) lua::ClearLog();

    Section("Scripts");
    ImGui::InputText("Path", s_path, sizeof(s_path));
    if (ImGui::Button("Run File")) lua::RunFile(s_path);
    ImGui::SameLine();
    if (ImGui::Button("Dump players")) {
        lua::RunString(
            "for i,p in ipairs(cheat.players()) do\n"
            "  cheat.log(i, p.name, string.format('%.0fm', p.dist), p.health)\n"
            "end");
    }

    Section("API Reference");
    ImGui::TextWrapped(
        "cheat.base()  cheat.local_player()  cheat.screen_w()/screen_h()\n"
        "cheat.read8/16/32/64/f/d(addr)   cheat.write8/32/f/d(addr, val)\n"
        "cheat.world_to_screen(x,y,z) -> sx, sy, visible\n"
        "cheat.players() -> { {addr, name, health, dist, x, y, z}, ... }\n"
        "cheat.draw_line(x1,y1,x2,y2,r,g,b,a,thick)\n"
        "cheat.draw_rect(x1,y1,x2,y2,r,g,b,a,thick)\n"
        "cheat.draw_text(x,y,text,r,g,b,a)\n"
        "cheat.draw_circle(x,y,radius,r,g,b,a,thick)\n"
        "cheat.log(...)");
}

static void DrawSettings() {
    Section("Config");
    ImGui::InputText("Name", s_cfgName, sizeof(s_cfgName));
    if (ImGui::Button("Save", ImVec2(100, 0))) {
        if (config::Save(s_cfgName)) notify::Push("Config saved: " + std::string(s_cfgName));
        else notify::Push("Save failed", palette::Bad);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load", ImVec2(100, 0))) {
        if (config::Load(s_cfgName)) notify::Push("Config loaded: " + std::string(s_cfgName));
        else notify::Push("Config not found", palette::Bad);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset", ImVec2(100, 0))) {
        config::Reset();
        notify::Push("Config reset");
    }

    Section("About");
    ImGui::TextWrapped(
        "VENICE EXTERNAL for FiveM.\n"
        "Fully external: no injection, no hooks. Reads FiveM_GTAProcess.exe memory and "
        "renders on a transparent topmost D3D11 overlay.\n\n"
        "Offsets live in src/core/offsets.h. When FiveM updates, retune them from the "
        "MISC tab without recompiling.");
    ImGui::Spacing();
    ImGui::Text("Menu Key: %s", KeyName(cfg.misc.menuKey));
}

void DrawLuaOverlay() {
    for (auto& d : lua::ConsumeDraw()) {
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImU32 c = IM_COL32((int)(d.r * 255), (int)(d.g * 255), (int)(d.bl * 255), (int)(d.al * 255));
        switch (d.type) {
        case 0: dl->AddLine(ImVec2(d.a, d.b), ImVec2(d.c, d.d), c, d.thick); break;
        case 1: dl->AddRect(ImVec2(d.a, d.b), ImVec2(d.c, d.d), c, 0.f, 0, d.thick); break;
        case 2: render::TextOutlined(dl, ImVec2(d.a, d.b), d.text.c_str(), c, false); break;
        case 3: dl->AddCircle(ImVec2(d.a, d.b), d.c, c, 48, d.thick); break;
        }
    }
}

void Draw() {
    if (s_bindDelay > 0.f) s_bindDelay -= ImGui::GetIO().DeltaTime;
    if (!g::menuOpen) return;

    ImGui::SetNextWindowSize(ImVec2(760, 520), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(620, 400), ImVec2(1400, 1000));

    ImGui::Begin("##venice", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wp = ImGui::GetWindowPos();
    ImVec2 ws = ImGui::GetWindowSize();
    float barH = 42.f;

    dl->AddRectFilledMultiColor(
        wp, ImVec2(wp.x + ws.x, wp.y + barH),
        IM_COL32(255, 62, 150, 60), IM_COL32(10, 10, 14, 200),
        IM_COL32(10, 10, 14, 200), IM_COL32(255, 62, 150, 60));

    ImGui::PushFont(theme::fontBold);
    dl->AddText(ImVec2(wp.x + 16, wp.y + 12), IM_COL32(255, 255, 255, 255), "VENICE");
    dl->AddText(ImVec2(wp.x + 92, wp.y + 12), IM_COL32(255, 62, 150, 255), "EXTERNAL");
    ImGui::PopFont();

    char status[96];
    snprintf(status, sizeof(status), "%s  |  %.0f fps  |  %d players",
             g::attached ? "ATTACHED" : "SEARCHING", g::fps, g::playerCount);
    ImVec2 st = ImGui::CalcTextSize(status);
    dl->AddText(ImVec2(wp.x + ws.x - st.x - 16, wp.y + 14),
                g::attached ? IM_COL32(74, 222, 128, 255) : IM_COL32(255, 180, 70, 255), status);

    ImGui::SetCursorPosY(barH + 6);

    ImGui::BeginChild("##side", ImVec2(150, 0), true);
    for (int i = 0; i < IM_ARRAYSIZE(kTabs); i++) {
        bool sel = (s_tab == i);
        if (sel) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, .24f, .59f, .22f));
        if (ImGui::Button(kTabs[i], ImVec2(-1, 34))) s_tab = i;
        if (sel) {
            ImGui::PopStyleColor();
            ImVec2 a = ImGui::GetItemRectMin();
            ImVec2 b = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(a.x, a.y), ImVec2(a.x + 3, b.y), IM_COL32(255, 62, 150, 255), 2.f);
        }
        ImGui::Spacing();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##content", ImVec2(0, 0), false);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 7));

    switch (s_tab) {
    case 0: DrawAimbot();   break;
    case 1: DrawEsp();      break;
    case 2: DrawVisuals();  break;
    case 3: DrawMisc();     break;
    case 4: DrawLua();      break;
    case 5: DrawSettings(); break;
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::End();
}

} // namespace menu