#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
#include "math.h"

namespace g {
    inline HWND      overlayWindow = nullptr;
    inline HWND      gameWindow    = nullptr;
    inline HINSTANCE instance      = nullptr;

    inline int  screenW = 1920;
    inline int  screenH = 1080;

    inline bool running   = true;
    inline bool menuOpen  = true;
    inline bool attached  = false;
    inline float fps      = 0.f;
    inline int  playerCount = 0;

    // live-tunable offset overrides (Debug tab)
    inline int  boneMode      = 0;      // 0 = fragInst+0x1A0, 1 = fragInst+0x20, 2 = fragInst direct
    inline int  boneStride    = 0x70;
    inline int  bonePosOff    = 0x10;
    inline int  viewMatrixSrc = -1;
}

#define AIM_FIELDS(X)                          \
    X(bool,  enabled,          false)          \
    X(int,   key,              VK_XBUTTON2)    \
    X(int,   mode,             0)              \
    X(float, fov,              70.f)           \
    X(bool,  drawFov,          true)           \
    X(bool,  fovFilled,        false)          \
    X(float, smooth,           3.5f)           \
    X(bool,  accelCurve,       true)           \
    X(int,   hitbox,           0)              \
    X(float, maxDistance,      300.f)          \
    X(bool,  prediction,       true)           \
    X(float, predictionAmount, 1.0f)           \
    X(bool,  visibleOnly,      false)          \
    X(bool,  ignoreDead,       true)           \
    X(bool,  ignoreLocal,      true)           \
    X(bool,  ignoreNpc,        true)           \
    X(int,   priority,         0)              \
    X(bool,  lockTarget,       true)           \
    X(float, switchDelay,      0.15f)          \
    X(int,   aimMethod,        0)              \
    X(float, maxStep,          450.f)          \
    X(bool,  drawTarget,       true)           \
    X(bool,  drawLineToTarget, true)           \
    X(bool,  triggerbot,       false)          \
    X(int,   triggerKey,       0)              \
    X(float, triggerDelay,     0.03f)          \
    X(float, triggerRadius,    2.0f)           \
    X(RGBA,  color,            RGBA(1.f, .24f, .59f, .85f))

#define ESP_FIELDS(X)                          \
    X(bool,  enabled,       true)              \
    X(bool,  boxes,         true)              \
    X(int,   boxStyle,      0)                 \
    X(float, boxThickness,  1.5f)              \
    X(bool,  boxFill,       false)             \
    X(float, boxFillAlpha,  0.14f)             \
    X(bool,  skeleton,      true)              \
    X(float, skelThickness, 1.5f)              \
    X(bool,  names,         true)              \
    X(bool,  health,        true)              \
    X(bool,  armor,         false)             \
    X(bool,  distance,      true)              \
    X(bool,  weapons,       false)             \
    X(bool,  snaplines,     false)             \
    X(int,   snapOrigin,    2)                 \
    X(bool,  headDot,       false)             \
    X(float, maxDistance,   500.f)             \
    X(bool,  textOutline,   true)              \
    X(bool,  hideLocal,     true)              \
    X(RGBA,  visibleColor,  RGBA(1.f, .24f, .59f, 1.f)) \
    X(RGBA,  invisibleColor,RGBA(1.f, .35f, .35f, .80f)) \
    X(RGBA,  skelColor,     RGBA(1.f, .48f, .72f, .90f)) \
    X(RGBA,  nameColor,     RGBA(.93f, .93f, .95f, 1.f)) \
    X(RGBA,  healthColor,   RGBA(.29f, .87f, .50f, 1.f)) \
    X(RGBA,  armorColor,    RGBA(.35f, .65f, 1.f, 1.f))  \
    X(RGBA,  distColor,     RGBA(.65f, .65f, .72f, 1.f))

#define VEH_FIELDS(X)                          \
    X(bool,  enabled,      false)              \
    X(bool,  box,          true)               \
    X(bool,  name,         true)               \
    X(bool,  distance,     true)               \
    X(bool,  speed,        true)               \
    X(float, maxDistance,  400.f)              \
    X(RGBA,  color,        RGBA(1.f, .24f, .59f, .9f))

#define MISC_FIELDS(X)                         \
    X(bool,  watermark,      true)             \
    X(bool,  fpsCounter,     true)             \
    X(bool,  playerCounter,  true)             \
    X(bool,  crosshair,      false)            \
    X(float, crosshairSize,  8.f)              \
    X(bool,  crosshairDot,   true)             \
    X(bool,  fovChanger,     false)            \
    X(float, gameFov,        78.f)             \
    X(bool,  speedometer,    false)            \
    X(bool,  boneDebug,      false)            \
    X(int,   menuKey,        VK_INSERT)        \
    X(RGBA,  watermarkColor, RGBA(1.f, .24f, .59f, .9f))

struct AimbotSettings {
#define X(t,n,d) t n = d;
    AIM_FIELDS(X)
#undef X
};
struct EspSettings {
#define X(t,n,d) t n = d;
    ESP_FIELDS(X)
#undef X
};
struct VehicleSettings {
#define X(t,n,d) t n = d;
    VEH_FIELDS(X)
#undef X
};
struct MiscSettings {
#define X(t,n,d) t n = d;
    MISC_FIELDS(X)
#undef X
};

struct Config {
    AimbotSettings  aim;
    EspSettings     esp;
    VehicleSettings veh;
    MiscSettings    misc;
    bool menuOpenState = true;
};

inline Config cfg;