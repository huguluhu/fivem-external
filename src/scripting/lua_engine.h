#pragma once
#include <string>
#include <vector>

namespace lua {
    bool Init();
    void Shutdown();
    void RunString(const std::string& code);
    void RunFile(const std::string& path);
    std::vector<std::string> Snapshot();
    void ClearLog();

    struct DrawCmd {
        int type;
        float a, b, c, d;
        float r, g, bl, al;
        float thick;
        std::string text;
    };
    std::vector<DrawCmd> ConsumeDraw();
}