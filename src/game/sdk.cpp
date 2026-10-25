#include "sdk.h"
#include "core/memory.h"
#include "core/offsets.h"
#include "core/globals.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <unordered_map>

namespace sdk {

std::vector<Player> players;
Vec3      localPos;
uintptr_t localPed = 0;
bool      localValid = false;
Matrix4   viewMatrix;
int       viewMatrixSource = -1;

static uintptr_t s_world = 0;
static uintptr_t s_replay = 0;

// A valid column-major view matrix has m[3]=m[7]=m[11]=0 and m[15]=1.
static bool Plausible(const Matrix4& m) {
    for (int i = 0; i < 16; i++)
        if (!std::isfinite(m.m[i]) || std::fabs(m.m[i]) > 1.0e6f) return false;
    if (std::fabs(m.m[3]) > 0.05f)  return false;
    if (std::fabs(m.m[7]) > 0.05f)  return false;
    if (std::fabs(m.m[11]) > 0.05f) return false;
    if (std::fabs(m.m[15] - 1.f) > 0.05f) return false;
    return true;
}

void ResolveViewMatrix() {
    if (viewMatrixSource >= 0) {
        if (viewMatrixSource == 0) {
            Matrix4 probe = mem::Read<Matrix4>(mem::g_base + off::CameraCandidate);
            if (Plausible(probe)) { viewMatrix = probe; return; }
        }
        uintptr_t cam = mem::ReadPtr(mem::g_base + off::CameraCandidate);
        if (cam) {
            Matrix4 a = mem::Read<Matrix4>(cam + off::ViewMatrixInCam);
            if (viewMatrixSource == 1 && Plausible(a)) { viewMatrix = a; return; }
            Matrix4 b = mem::Read<Matrix4>(cam);
            if (viewMatrixSource == 2 && Plausible(b)) { viewMatrix = b; return; }
        }
    }

    Matrix4 direct = mem::Read<Matrix4>(mem::g_base + off::CameraCandidate);
    if (Plausible(direct)) { viewMatrix = direct; viewMatrixSource = 0; return; }

    uintptr_t cam = mem::ReadPtr(mem::g_base + off::CameraCandidate);
    if (cam) {
        Matrix4 a = mem::Read<Matrix4>(cam + off::ViewMatrixInCam);
        if (Plausible(a)) { viewMatrix = a; viewMatrixSource = 1; return; }
        Matrix4 b = mem::Read<Matrix4>(cam);
        if (Plausible(b)) { viewMatrix = b; viewMatrixSource = 2; return; }
    }
    viewMatrixSource = -1;
}

bool Init() {
    if (!mem::g_base) return false;
    s_world  = mem::ReadPtr(mem::g_base + off::World);
    s_replay = mem::ReadPtr(mem::g_base + off::ReplayInterface);
    ResolveViewMatrix();
    return s_world != 0;
}

void Shutdown() { players.clear(); }

uintptr_t LocalPlayer() {
    if (!s_world) s_world = mem::ReadPtr(mem::g_base + off::World);
    return mem::ReadPtr(s_world + off::WorldLocalPlayer);
}

bool WorldToScreen(const Vec3& w, Vec2& out) {
    const float* m = viewMatrix.m;
    float cw = m[3] * w.x + m[7] * w.y + m[11] * w.z + m[15];
    if (cw < 0.001f) return false;
    float cx = m[0] * w.x + m[4] * w.y + m[8]  * w.z + m[12];
    float cy = m[1] * w.x + m[5] * w.y + m[9]  * w.z + m[13];
    float inv = 1.f / cw;
    out.x = (g::screenW * 0.5f) * (1.f + cx * inv);
    out.y = (g::screenH * 0.5f) * (1.f - cy * inv);
    return true;
}

Vec3 GetBonePos(uintptr_t ped, int bone) {
    uintptr_t frag = mem::ReadPtr(ped + off::EntityFragInst);
    if (!frag) return {};
    uintptr_t arr = (g::boneMode == 2) ? frag
                  : mem::ReadPtr(frag + (g::boneMode == 1 ? 0x20 : 0x1A0));
    if (!arr) return {};
    return mem::Read<Vec3>(arr + (size_t)bone * g::boneStride + g::bonePosOff);
}

static const std::unordered_map<uint32_t, const char*> kWeapons = {
    {0x1B06D571,"Pistol"},{0x5EF9FEC4,"Combat Pistol"},{0x22D8FE39,"AP Pistol"},
    {0x2BE6766B,"SMG"},{0xEFE7E2DF,"Assault SMG"},{0x1BC4FDB9,"Micro SMG"},
    {0x42BF8A85,"MG"},{0xDBBD7280,"Combat MG"},{0xBFEFFF6D,"Assault Rifle"},
    {0x394F415C,"Carbine Rifle"},{0x83BF0278,"Advanced Rifle"},{0x0C472FE2,"Special Carbine"},
    {0x7F38D042,"Bullpup Rifle"},{0x1D073A89,"Pump Shotgun"},{0x7846A318,"Assault Shotgun"},
    {0xE96C4C40,"Heavy Shotgun"},{0x05FC3C11,"Heavy Sniper"},{0x9D61E50C,"RPG"},
    {0xB1CA77B1,"Grenade Launcher"},{0x6E7DD9A7,"Combat PDW"},{0xA3C0C5F9,"Compact Rifle"}
};

std::string WeaponName(uint32_t hash) {
    auto it = kWeapons.find(hash);
    if (it != kWeapons.end()) return it->second;
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%08X", hash);
    return buf;
}

void Update() {
    players.clear();
    localPed = LocalPlayer();
    localValid = localPed != 0;

    if (localValid) localPos = mem::Read<Vec3>(localPed + off::EntityPosition);
    else            localPos = {};

    ResolveViewMatrix();

    if (!s_replay) s_replay = mem::ReadPtr(mem::g_base + off::ReplayInterface);
    if (!s_replay) return;

    uintptr_t iface = mem::ReadPtr(s_replay + off::ReplayPlayerIface);
    if (!iface) return;
    uintptr_t list = mem::ReadPtr(iface + off::PlayerIfaceList);
    int count = mem::Read<int>(iface + off::PlayerIfaceCount);
    if (!list || count <= 0) return;
    count = std::clamp(count, 0, 256);

    for (int i = 0; i < count; i++) {
        uintptr_t ped = mem::ReadPtr(list + (size_t)i * 8);
        if (!ped) continue;

        float hp = mem::Read<float>(ped + off::EntityHealth);
        if (!std::isfinite(hp) || hp > 100000.f) continue;

        Player p;
        p.ped        = ped;
        p.health     = hp;
        p.maxHealth  = mem::Read<float>(ped + off::EntityMaxHealth);
        if (!std::isfinite(p.maxHealth) || p.maxHealth <= 0.f) p.maxHealth = 200.f;
        p.armor      = mem::Read<float>(ped + off::EntityArmor);
        p.pos        = mem::Read<Vec3>(ped + off::EntityPosition);
        p.velocity   = mem::Read<Vec3>(ped + off::EntityVelocity);
        p.dead       = hp <= 0.f;
        p.invisible  = (mem::Read<uint8_t>(ped + off::EntityFlags) & 1) != 0;
        p.playerInfo = mem::ReadPtr(ped + off::EntityPlayerInfo);

        if (p.playerInfo)
            p.name = mem::ReadString(p.playerInfo + off::PlayerInfoName, 32);

        uintptr_t wm = mem::ReadPtr(ped + off::EntityWeaponMgr);
        if (wm) {
            uintptr_t wep = mem::ReadPtr(wm + off::WeaponMgrCurrent);
            if (wep) p.weaponHash = mem::Read<uint32_t>(wep + off::WeaponHash);
        }

        if (!std::isfinite(p.pos.x) || std::fabs(p.pos.x) > 100000.f) continue;

        p.distance = localValid ? Dist(p.pos, localPos) : 0.f;
        p.valid = true;
        players.push_back(std::move(p));
    }
}

} // namespace sdk