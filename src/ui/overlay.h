#pragma once
#include <windows.h>

namespace overlay {
    bool Create(HINSTANCE inst, const wchar_t* title);
    void Destroy();
    bool BeginFrame();
    void EndFrame();
    void SetClickThrough(bool through);
    void FollowGameWindow();
    bool PumpMessages();
    void* GetHwnd();
}