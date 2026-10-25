#pragma once
#include <cstdint>

// Offsets for FiveM_GTAProcess.exe. These drift with game builds.
// The Debug tab in the menu lets you live-tune the risky ones.
namespace off {

    // ---- module globals (from module base) ----
    constexpr uintptr_t World            = 0x25B14A0;  // CWorld*
    constexpr uintptr_t ReplayInterface  = 0x1FEAAC0;  // CReplayInterface*
    constexpr uintptr_t CameraCandidate  = 0x24C0B00;  // camera / viewport candidate

    // ---- CWorld ----
    constexpr uintptr_t WorldLocalPlayer = 0x08;   // CPed*
    constexpr uintptr_t WorldPlayerList  = 0x18;   // CPlayerList*

    // ---- CReplayInterface ----
    constexpr uintptr_t ReplayPlayerIface  = 0x18; // CPlayerInterface*
    constexpr uintptr_t ReplayVehicleIface = 0x20; // CVehicleInterface*

    constexpr uintptr_t PlayerIfaceList   = 0x100; // CPed** array
    constexpr uintptr_t PlayerIfaceCount  = 0x108; // int32
    constexpr uintptr_t VehicleIfaceList  = 0x180; // CVehicle** array
    constexpr uintptr_t VehicleIfaceCount = 0x188; // int32

    // ---- CPed / CEntity ----
    constexpr uintptr_t EntityPosition   = 0x90;   // Vec3
    constexpr uintptr_t EntityVelocity   = 0x30;   // Vec3
    constexpr uintptr_t EntityHealth     = 0x280;  // float
    constexpr uintptr_t EntityMaxHealth  = 0x2A0;  // float
    constexpr uintptr_t EntityArmor      = 0x150C; // float
    constexpr uintptr_t EntityFlags      = 0x2E;   // uint8 (bit0 = invisible)
    constexpr uintptr_t EntityFragInst   = 0x430;  // CFragInst*
    constexpr uintptr_t EntityPlayerInfo = 0x10A8; // CPlayerInfo*
    constexpr uintptr_t EntityWeaponMgr  = 0x10B8; // CWeaponManager*
    constexpr uintptr_t EntityVehicle    = 0x0D30; // CVehicle*

    // ---- CPlayerInfo ----
    constexpr uintptr_t PlayerInfoName   = 0x40;   // char[32]

    // ---- CWeaponManager -> CWeapon ----
    constexpr uintptr_t WeaponMgrCurrent = 0x20;
    constexpr uintptr_t WeaponHash       = 0x60;

    // ---- camera / view matrix candidates (auto-probed at runtime) ----
    constexpr uintptr_t ViewMatrixInCam  = 0x24C;  // float[16] inside camera object
    constexpr uintptr_t CameraFov        = 0x30;   // float
}