#pragma once
#include "imgui.h"

namespace theme {
    void Apply();
    void LoadFonts(float dpiScale);
    extern ImFont* fontRegular;
    extern ImFont* fontBold;
    extern ImFont* fontMono;
}