#include "render.h"
#include <algorithm>

namespace render {

void TextOutlined(ImDrawList* dl, ImVec2 pos, const char* text, ImU32 col, bool outline) {
    if (outline) {
        ImU32 shadow = IM_COL32(0, 0, 0, 210);
        dl->AddText(ImVec2(pos.x - 1, pos.y), shadow, text);
        dl->AddText(ImVec2(pos.x + 1, pos.y), shadow, text);
        dl->AddText(ImVec2(pos.x, pos.y - 1), shadow, text);
        dl->AddText(ImVec2(pos.x, pos.y + 1), shadow, text);
    }
    dl->AddText(pos, col, text);
}

void TextCentered(ImDrawList* dl, float cx, float y, const char* text, ImU32 col, bool outline) {
    ImVec2 sz = ImGui::CalcTextSize(text);
    TextOutlined(dl, ImVec2(cx - sz.x * 0.5f, y), text, col, outline);
}

void CornerBox(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float thick, float len) {
    len = std::min(len, std::min(b.x - a.x, b.y - a.y) * 0.5f);
    dl->AddLine(ImVec2(a.x, a.y), ImVec2(a.x + len, a.y), col, thick);
    dl->AddLine(ImVec2(a.x, a.y), ImVec2(a.x, a.y + len), col, thick);
    dl->AddLine(ImVec2(b.x, a.y), ImVec2(b.x - len, a.y), col, thick);
    dl->AddLine(ImVec2(b.x, a.y), ImVec2(b.x, a.y + len), col, thick);
    dl->AddLine(ImVec2(a.x, b.y), ImVec2(a.x + len, b.y), col, thick);
    dl->AddLine(ImVec2(a.x, b.y), ImVec2(a.x, b.y - len), col, thick);
    dl->AddLine(ImVec2(b.x, b.y), ImVec2(b.x - len, b.y), col, thick);
    dl->AddLine(ImVec2(b.x, b.y), ImVec2(b.x, b.y - len), col, thick);
}

void HealthBar(ImDrawList* dl, ImVec2 a, ImVec2 b, float pct, ImU32 col) {
    pct = Clampf(pct, 0.f, 1.f);
    float h = (b.y - a.y) * pct;
    dl->AddRectFilled(ImVec2(a.x, b.y - h), b, col);
    dl->AddRect(a, b, IM_COL32(0, 0, 0, 200), 0.f, 0, 1.f);
}

} // namespace render