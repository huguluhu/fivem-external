#include <windows.h>
#include <chrono>
#include <cstdio>
#include <cstdarg>
#include <string>
#include "core/globals.h"
#include "core/memory.h"
#include "game/sdk.h"
#include "features/aimbot.h"
#include "features/esp.h"
#include "features/misc.h"
#include "ui/overlay.h"
#include "ui/menu.h"
#include "ui/notify.h"
#include "config/config.h"
#include "scripting/lua_engine.h"
#include "imgui.h"

static bool  s_menuKeyHeld = false;
static FILE* s_log = nullptr;

static void Log(const char* fmt, ...) {
    if (!s_log) return;
    va_list ap; va_start(ap, fmt);
    vfprintf(s_log, fmt, ap);
    va_end(ap);
    fputc('\n', s_log);
    fflush(s_log);
}

static void OpenLog() {
    char path[MAX_PATH]{};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string p(path);
    size_t s = p.find_last_of("\\/");
    if (s != std::string::npos) p = p.substr(0, s + 1);
    p += "venice_log.txt";
    s_log = fopen(p.c_str(), "w");
    Log("=== Venice External started ===");
}

static HWND s_bestWindow = nullptr;
static long s_bestArea = 0;

static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM) {
    if (hwnd == g::overlayWindow) return TRUE;
    if (!IsWindowVisible(hwnd)) return TRUE;
    if (GetWindow(hwnd, GW_OWNER) != nullptr) return TRUE;

    RECT r{};
    GetClientRect(hwnd, &r);
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w < 480 || h < 270) return TRUE;

    bool isGame = false;

    wchar_t cls[256]{};
    GetClassNameW(hwnd, cls, 256);
    if (_wcsicmp(cls, L"grcWindow") == 0 || _wcsicmp(cls, L"CfxWindow") == 0)
        isGame = true;

    if (!isGame) {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid) {
            HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
            if (h) {
                wchar_t path[MAX_PATH]{};
                DWORD sz = MAX_PATH;
                if (QueryFullProcessImageNameW(h, 0, path, &sz)) {
                    std::wstring wp(path);
                    if (wp.find(L"FiveM.exe") != std::wstring::npos ||
                        wp.find(L"FiveM_GTAProcess.exe") != std::wstring::npos)
                        isGame = true;
                }
                CloseHandle(h);
            }
        }
    }

    if (!isGame) return TRUE;

    long area = (long)w * h;
    if (area > s_bestArea) { s_bestArea = area; s_bestWindow = hwnd; }
    return TRUE;
}

static void FindGameWindow() {
    s_bestWindow = nullptr;
    s_bestArea = 0;
    EnumWindows(EnumProc, 0);
    if (s_bestWindow && s_bestWindow != g::gameWindow) {
        g::gameWindow = s_bestWindow;
        Log("game window found hwnd=0x%p", (void*)g::gameWindow);
    }
}

static void ToggleMenu() {
    g::menuOpen = !g::menuOpen;
    overlay::SetClickThrough(!g::menuOpen);
    Log("menu %s", g::menuOpen ? "OPEN" : "CLOSED");

    if (g::menuOpen) {
        SetForegroundWindow((HWND)overlay::GetHwnd());
    } else if (g::gameWindow) {
        SetForegroundWindow(g::gameWindow);
    }
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int) {
    OpenLog();

    if (!overlay::Create(inst, L"Venice External")) {
        Log("overlay::Create FAILED");
        MessageBoxA(nullptr,
            "Failed to create the DirectX overlay.\n"
            "Update your GPU drivers and try again.",
            "Venice External", MB_ICONERROR);
        return 1;
    }
    g::overlayWindow = (HWND)overlay::GetHwnd();
    Log("overlay created hwnd=0x%p", (void*)g::overlayWindow);

    if (!mem::Attach(L"FiveM_GTAProcess.exe")) {
        Log("attach failed, will retry");
        notify::Push("Waiting for FiveM_GTAProcess.exe ...", palette::Bad);
    } else {
        g::attached = true;
        Log("attached pid=%lu base=0x%llX", mem::g_pid, (unsigned long long)mem::g_base);
        notify::Push("Attached to FiveM", palette::Good);
    }

    sdk::Init();
    lua::Init();
    config::Load("default");

    g::menuOpen = true;
    overlay::SetClickThrough(false);
    notify::Push("Menu ready - press INSERT", palette::Accent);

    auto last = std::chrono::steady_clock::now();
    auto lastUpdate = last;
    auto lastRescan = last;

    while (g::running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (dt > 0.25f) dt = 0.25f;

        if (!overlay::PumpMessages()) break;

        bool keyDown = (GetAsyncKeyState(cfg.misc.menuKey) & 0x8000) != 0;
        if (keyDown && !s_menuKeyHeld && !menu::WantsKeybind()) ToggleMenu();
        s_menuKeyHeld = keyDown;

        if (std::chrono::duration<float>(now - lastRescan).count() > 2.f) {
            lastRescan = now;
            FindGameWindow();
            if (!mem::Alive()) {
                if (g::attached) Log("lost process");
                g::attached = false;
                if (mem::Attach(L"FiveM_GTAProcess.exe")) {
                    g::attached = true;
                    sdk::Init();
                    Log("reattached base=0x%llX", (unsigned long long)mem::g_base);
                    notify::Push("Reattached to FiveM", palette::Good);
                }
            }
        }

        overlay::FollowGameWindow();

        float sinceUpdate = std::chrono::duration<float>(now - lastUpdate).count();
        if (g::attached && mem::Alive() && sinceUpdate > 0.008f) {
            lastUpdate = now;
            sdk::Update();
            g::playerCount = (int)sdk::players.size();
        }

        if (g::attached && mem::Alive()) {
            aimbot::Tick(dt);
            misc::Tick(dt);
        }
        notify::Tick(dt);

        if (overlay::BeginFrame()) {
            esp::Draw();
            menu::DrawLuaOverlay();
            misc::Draw();
            menu::Draw();
            notify::Draw();
            overlay::EndFrame();
        }

        Sleep(1);
    }

    Log("shutting down");
    config::Save("default");
    lua::Shutdown();
    overlay::Destroy();
    mem::Detach();
    return 0;
}