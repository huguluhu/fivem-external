#pragma once
#include "imgui.h"
#include "core/math.h"

namespace render {
    inline ImU32 U32(const RGBA& c) {
        return IM_COL32((int)(c.r*255), (int)(c.g*255), (int)(c.b*255), (int)(c.a*255));
    }
    inline ImVec4 V4(const RGBA& c) { return ImVec4(c.r, c.g, c.b, c.a); }

    void TextOutlined(ImDrawList* dl, ImVec2 pos, const char* text, ImU32 col, bool outline = true);
    void TextCentered(ImDrawList* dl, float cx, float y, const char* text, ImU32 col, bool outline = true);
    void CornerBox(ImDrawList* dl, ImVec2 min, ImVec2 max, ImU32 col, float thick, float cornerLen);
    void HealthBar(ImDrawList* dl, ImVec2 min, ImVec2 max, float pct, ImU32 col);
}