#include "notify.h"
#include "imgui.h"
#include "render.h"
#include "core/globals.h"
#include <deque>

namespace notify {

struct Item { std::string text; RGBA color; float life; };
static std::deque<Item> s_items;

void Push(const std::string& t, RGBA c) { s_items.push_back({ t, c, 3.0f }); }

void Tick(float dt) {
    for (auto& i : s_items) i.life -= dt;
    while (!s_items.empty() && s_items.front().life <= 0.f) s_items.pop_front();
}

void Draw() {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float y = 60.f;
    for (auto& i : s_items) {
        float a = Clampf(i.life, 0.f, 1.f);
        ImVec2 sz = ImGui::CalcTextSize(i.text.c_str());
        ImVec2 p(20.f, y);
        RGBA bg(0.06f, 0.06f, 0.08f, 0.85f * a);
        dl->AddRectFilled(ImVec2(p.x - 8, p.y - 4), ImVec2(p.x + sz.x + 8, p.y + sz.y + 4),
                          render::U32(bg), 4.f);
        RGBA accent = i.color; accent.a = a;
        dl->AddRectFilled(ImVec2(p.x - 8, p.y - 4), ImVec2(p.x - 6, p.y + sz.y + 4),
                          render::U32(accent), 4.f);
        RGBA tc(0.93f, 0.93f, 0.95f, a);
        render::TextOutlined(dl, p, i.text.c_str(), render::U32(tc), false);
        y += sz.y + 16.f;
    }
}

} // namespace notify