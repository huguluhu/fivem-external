#include "overlay.h"
#include "core/globals.h"
#include "theme.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dwmapi.h>
#include <cstring>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace overlay {

static ID3D11Device*           s_device = nullptr;
static ID3D11DeviceContext*    s_ctx    = nullptr;
static IDXGISwapChain1*        s_swap   = nullptr;
static ID3D11RenderTargetView* s_rtv    = nullptr;
static HWND                    s_hwnd   = nullptr;
static RECT                    s_lastRect{};
static bool s_shown = false;

static void CreateRTV() {
    if (s_rtv) { s_rtv->Release(); s_rtv = nullptr; }
    ID3D11Texture2D* back = nullptr;
    if (SUCCEEDED(s_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back)) && back) {
        s_device->CreateRenderTargetView(back, nullptr, &s_rtv);
        back->Release();
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w, l)) return true;

    switch (msg) {
    case WM_SIZE:
        if (s_device && w != SIZE_MINIMIZED) CreateRTV();
        return 0;
    case WM_SYSCOMMAND:
        if ((w & 0xFFF0) == SC_KEYMENU) return 0;
        break;
    case WM_MOUSEACTIVATE:
        return MA_ACTIVATE;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, w, l);
}

bool Create(HINSTANCE inst, const wchar_t* title) {
    g::instance = inst;
    ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = inst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"VeniceOverlayClass";
    RegisterClassExW(&wc);

    s_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName, title, WS_POPUP,
        0, 0, 1920, 1080, nullptr, nullptr, inst, nullptr);
    if (!s_hwnd) return false;

    MARGINS m{ -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(s_hwnd, &m);
    SetLayeredWindowAttributes(s_hwnd, 0, 255, LWA_ALPHA);

    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.Width       = 1920;
    sd.Height      = 1080;
    sd.Format      = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.SampleDesc  = { 1, 0 };
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.Scaling     = DXGI_SCALING_STRETCH;
    sd.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode   = DXGI_ALPHA_MODE_PREMULTIPLIED;

    D3D_FEATURE_LEVEL fl;
    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                 levels, 2, D3D11_SDK_VERSION,
                                 &s_device, &fl, &s_ctx)))
        return false;

    IDXGIDevice* dxgiDev = nullptr;
    s_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDev);
    IDXGIAdapter* adapter = nullptr;
    dxgiDev->GetAdapter(&adapter);
    IDXGIFactory2* factory = nullptr;
    adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);

    factory->CreateSwapChainForHwnd(s_device, s_hwnd, &sd, nullptr, nullptr, &s_swap);
    factory->MakeWindowAssociation(s_hwnd, DXGI_MWA_NO_ALT_ENTER);

    adapter->Release(); dxgiDev->Release(); factory->Release();

    CreateRTV();
    SetClickThrough(true);

    if (!ImGui_ImplWin32_Init(s_hwnd)) return false;
    if (!ImGui_ImplDX11_Init(s_device, s_ctx)) return false;

    float dpi = ImGui_ImplWin32_GetDpiScaleForHwnd(s_hwnd);
    if (dpi <= 0.f) dpi = 1.f;
    theme::LoadFonts(dpi);
    theme::Apply();
    return true;
}

void Destroy() {
    if (s_rtv) s_rtv->Release();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (s_swap) s_swap->Release();
    if (s_ctx)  s_ctx->Release();
    if (s_device) s_device->Release();
    if (s_hwnd) DestroyWindow(s_hwnd);
}

void SetClickThrough(bool through) {
    LONG ex = GetWindowLongW(s_hwnd, GWL_EXSTYLE);
    if (through) ex |= WS_EX_TRANSPARENT; else ex &= ~WS_EX_TRANSPARENT;
    ex |= WS_EX_LAYERED | WS_EX_TOPMOST;
    SetWindowLongW(s_hwnd, GWL_EXSTYLE, ex);
}

void FollowGameWindow() {
    RECT target{};
    bool ok = false;

    if (g::gameWindow && IsWindow(g::gameWindow) &&
        IsWindowVisible(g::gameWindow) && !IsIconic(g::gameWindow)) {
        RECT cr{};
        GetClientRect(g::gameWindow, &cr);
        POINT tl{ cr.left, cr.top };
        ClientToScreen(g::gameWindow, &tl);
        int w = cr.right - cr.left;
        int h = cr.bottom - cr.top;
        if (w > 0 && h > 0) {
            target.left = tl.x;   target.top = tl.y;
            target.right = tl.x + w; target.bottom = tl.y + h;
            ok = true;
        }
    }

    if (!ok) {
        target.left = 0; target.top = 0;
        target.right = GetSystemMetrics(SM_CXSCREEN);
        target.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if (target.right - target.left <= 0 || target.bottom - target.top <= 0) return;

    if (!s_shown || memcmp(&target, &s_lastRect, sizeof(RECT)) != 0) {
        s_lastRect = target;
        SetWindowPos(s_hwnd, HWND_TOPMOST, target.left, target.top,
                     target.right - target.left, target.bottom - target.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        g::screenW = target.right - target.left;
        g::screenH = target.bottom - target.top;
        s_shown = true;
    }
}

bool PumpMessages() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
        if (msg.message == WM_QUIT) return false;
    }
    return true;
}

bool BeginFrame() {
    if (!s_device || !s_swap) return false;
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    return true;
}

void EndFrame() {
    ImGui::Render();
    const float clear[4] = { 0.f, 0.f, 0.f, 0.f };
    s_ctx->OMSetRenderTargets(1, &s_rtv, nullptr);
    s_ctx->ClearRenderTargetView(s_rtv, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    s_swap->Present(1, 0);
}

void* GetHwnd() { return s_hwnd; }

} // namespace overlay