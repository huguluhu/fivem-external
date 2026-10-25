#pragma once
#include <string>
#include "core/math.h"

namespace notify {
    void Push(const std::string& text, RGBA color = palette::Accent);
    void Draw();
    void Tick(float dt);
}