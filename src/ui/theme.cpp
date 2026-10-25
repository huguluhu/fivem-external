#include "theme.h"
#include "core/math.h"
#include <windows.h>

namespace theme {

ImFont* fontRegular = nullptr;
ImFont* fontBold    = nullptr;
ImFont* fontMono    = nullptr;

static ImVec4 C(const RGBA& c) { return ImVec4(c.r, c.g, c.b, c.a); }

void LoadFonts(float dpi) {
    ImGuiIO& io = ImGui::GetIO();
    const char* reg  = "C:\\Windows\\Fonts\\segoeui.ttf";
    const char* bold = "C:\\Windows\\Fonts\\segoeuib.ttf";
    const char* mono = "C:\\Windows\\Fonts\\consola.ttf";

    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;
    cfg.PixelSnapH = true;

    fontRegular = io.Fonts->AddFontFromFileTTF(reg,  16.f * dpi, &cfg);
    fontBold    = io.Fonts->AddFontFromFileTTF(bold, 17.f * dpi, &cfg);
    fontMono    = io.Fonts->AddFontFromFileTTF(mono, 14.f * dpi, &cfg);
    if (!fontRegular) fontRegular = io.Fonts->AddFontDefault();
    if (!fontBold)    fontBold    = fontRegular;
    if (!fontMono)    fontMono    = fontRegular;
    io.FontDefault = fontRegular;
}

void Apply() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 8.f;
    s.ChildRounding     = 6.f;
    s.FrameRounding     = 5.f;
    s.PopupRounding     = 6.f;
    s.ScrollbarRounding = 6.f;
    s.GrabRounding      = 5.f;
    s.TabRounding       = 6.f;
    s.WindowBorderSize  = 1.f;
    s.ChildBorderSize   = 1.f;
    s.FrameBorderSize   = 1.f;
    s.WindowPadding     = ImVec2(12, 12);
    s.FramePadding      = ImVec2(10, 6);
    s.ItemSpacing       = ImVec2(10, 8);
    s.ScrollbarSize     = 10.f;
    s.GrabMinSize       = 12.f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_Text]                  = C(palette::Text);
    c[ImGuiCol_TextDisabled]          = C(palette::TextDim);
    c[ImGuiCol_WindowBg]              = C(palette::Bg);
    c[ImGuiCol_ChildBg]               = ImVec4(0.055f, 0.055f, 0.071f, 1.f);
    c[ImGuiCol_PopupBg]               = C(palette::Panel);
    c[ImGuiCol_Border]                = C(palette::Border);
    c[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_FrameBg]               = C(palette::Widget);
    c[ImGuiCol_FrameBgHovered]        = ImVec4(0.16f, 0.16f, 0.20f, 1.f);
    c[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.20f, 0.25f, 1.f);
    c[ImGuiCol_TitleBg]               = C(palette::Panel);
    c[ImGuiCol_TitleBgActive]         = C(palette::Panel);
    c[ImGuiCol_MenuBarBg]             = C(palette::Panel);
    c[ImGuiCol_ScrollbarBg]           = ImVec4(0.05f, 0.05f, 0.06f, 1.f);
    c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.22f, 0.22f, 0.28f, 1.f);
    c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.30f, 0.30f, 0.37f, 1.f);
    c[ImGuiCol_ScrollbarGrabActive]   = C(palette::Accent);
    c[ImGuiCol_CheckMark]             = C(palette::Accent);
    c[ImGuiCol_SliderGrab]            = C(palette::Accent);
    c[ImGuiCol_SliderGrabActive]      = C(palette::AccentBright);
    c[ImGuiCol_Button]                = ImVec4(0.14f, 0.14f, 0.18f, 1.f);
    c[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.13f, 0.19f, 1.f);
    c[ImGuiCol_ButtonActive]          = C(palette::AccentDim);
    c[ImGuiCol_Header]                = ImVec4(0.18f, 0.11f, 0.16f, 1.f);
    c[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.14f, 0.21f, 1.f);
    c[ImGuiCol_HeaderActive]          = C(palette::AccentDim);
    c[ImGuiCol_Separator]             = C(palette::Border);
    c[ImGuiCol_SeparatorHovered]      = C(palette::AccentDim);
    c[ImGuiCol_SeparatorActive]       = C(palette::Accent);
    c[ImGuiCol_ResizeGrip]            = C(palette::AccentDim);
    c[ImGuiCol_ResizeGripHovered]     = C(palette::Accent);
    c[ImGuiCol_ResizeGripActive]      = C(palette::AccentBright);
    c[ImGuiCol_Tab]                   = ImVec4(0.09f, 0.09f, 0.12f, 1.f);
    c[ImGuiCol_TabHovered]            = ImVec4(0.24f, 0.13f, 0.19f, 1.f);
    c[ImGuiCol_TabSelected]           = C(palette::AccentDim);
    c[ImGuiCol_TabDimmed]             = ImVec4(0.07f, 0.07f, 0.09f, 1.f);
    c[ImGuiCol_TabDimmedSelected]     = ImVec4(0.16f, 0.10f, 0.14f, 1.f);
    c[ImGuiCol_PlotLines]             = C(palette::Accent);
    c[ImGuiCol_PlotHistogram]         = C(palette::Accent);
    c[ImGuiCol_TextSelectedBg]        = ImVec4(1.f, 0.24f, 0.59f, 0.35f);
    c[ImGuiCol_NavHighlight]          = C(palette::Accent);
}

} // namespace theme