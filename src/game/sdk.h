#pragma once
#include <string>
#include <vector>
#include "core/math.h"

struct Player {
    uintptr_t   ped = 0;
    uintptr_t   playerInfo = 0;
    std::string name;
    Vec3        pos;
    Vec3        velocity;
    float       health = 0.f;
    float       maxHealth = 200.f;
    float       armor = 0.f;
    uint32_t    weaponHash = 0;
    float       distance = 0.f;
    bool        valid = false;
    bool        dead = false;
    bool        invisible = false;
};

namespace sdk {
    extern std::vector<Player> players;
    extern Vec3     localPos;
    extern uintptr_t localPed;
    extern bool     localValid;
    extern Matrix4  viewMatrix;
    extern int      viewMatrixSource;

    bool Init();
    void Shutdown();
    void Update();

    uintptr_t LocalPlayer();
    void ResolveViewMatrix();
    bool WorldToScreen(const Vec3& world, Vec2& out);
    Vec3 GetBonePos(uintptr_t ped, int bone);
    std::string WeaponName(uint32_t hash);
}