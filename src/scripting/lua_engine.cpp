#include "lua_engine.h"
#include "core/globals.h"
#include "core/memory.h"
#include "game/sdk.h"
#include "core/math.h"
#include <cstdio>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace lua {

static lua_State* L = nullptr;
static std::vector<std::string> s_log;
static std::vector<DrawCmd> s_draw;

static void Log(const std::string& s) {
    if (s_log.size() > 500) s_log.erase(s_log.begin(), s_log.begin() + 100);
    s_log.push_back(s);
}

static int l_base(lua_State* l)  { lua_pushinteger(l, (lua_Integer)mem::g_base); return 1; }
static int l_local(lua_State* l) { lua_pushinteger(l, (lua_Integer)sdk::localPed); return 1; }
static int l_sw(lua_State* l)    { lua_pushinteger(l, g::screenW); return 1; }
static int l_sh(lua_State* l)    { lua_pushinteger(l, g::screenH); return 1; }

static int l_read8(lua_State* l)  { lua_pushinteger(l, mem::Read<uint8_t>((uintptr_t)luaL_checkinteger(l,1))); return 1; }
static int l_read16(lua_State* l) { lua_pushinteger(l, mem::Read<int16_t>((uintptr_t)luaL_checkinteger(l,1))); return 1; }
static int l_read32(lua_State* l) { lua_pushinteger(l, mem::Read<int32_t>((uintptr_t)luaL_checkinteger(l,1))); return 1; }
static int l_read64(lua_State* l) { lua_pushinteger(l, (lua_Integer)mem::Read<uint64_t>((uintptr_t)luaL_checkinteger(l,1))); return 1; }
static int l_readf(lua_State* l)  { lua_pushnumber(l, mem::Read<float>((uintptr_t)luaL_checkinteger(l,1))); return 1; }
static int l_readd(lua_State* l)  { lua_pushnumber(l, mem::Read<double>((uintptr_t)luaL_checkinteger(l,1))); return 1; }

static int l_write8(lua_State* l)  { mem::Write<uint8_t>((uintptr_t)luaL_checkinteger(l,1), (uint8_t)luaL_checkinteger(l,2)); return 0; }
static int l_write32(lua_State* l) { mem::Write<int32_t>((uintptr_t)luaL_checkinteger(l,1), (int32_t)luaL_checkinteger(l,2)); return 0; }
static int l_writef(lua_State* l)  { mem::Write<float>((uintptr_t)luaL_checkinteger(l,1), (float)luaL_checknumber(l,2)); return 0; }
static int l_writed(lua_State* l)  { mem::Write<double>((uintptr_t)luaL_checkinteger(l,1), luaL_checknumber(l,2)); return 0; }

static int l_w2s(lua_State* l) {
    Vec3 w((float)luaL_checknumber(l,1), (float)luaL_checknumber(l,2), (float)luaL_checknumber(l,3));
    Vec2 s;
    bool ok = sdk::WorldToScreen(w, s);
    lua_pushnumber(l, s.x);
    lua_pushnumber(l, s.y);
    lua_pushboolean(l, ok);
    return 3;
}

static int l_players(lua_State* l) {
    lua_newtable(l);
    int idx = 1;
    for (auto& p : sdk::players) {
        if (!p.valid) continue;
        lua_newtable(l);
        lua_pushstring(l, "addr");   lua_pushinteger(l, (lua_Integer)p.ped); lua_settable(l, -3);
        lua_pushstring(l, "name");   lua_pushstring(l, p.name.c_str());      lua_settable(l, -3);
        lua_pushstring(l, "health"); lua_pushnumber(l, p.health);            lua_settable(l, -3);
        lua_pushstring(l, "dist");   lua_pushnumber(l, p.distance);          lua_settable(l, -3);
        lua_pushstring(l, "x");      lua_pushnumber(l, p.pos.x);             lua_settable(l, -3);
        lua_pushstring(l, "y");      lua_pushnumber(l, p.pos.y);             lua_settable(l, -3);
        lua_pushstring(l, "z");      lua_pushnumber(l, p.pos.z);             lua_settable(l, -3);
        lua_rawseti(l, -2, idx++);
    }
    return 1;
}

static void pushColor(lua_State* l, DrawCmd& d, int base) {
    d.r  = (float)luaL_optnumber(l, base,     1.0);
    d.g  = (float)luaL_optnumber(l, base + 1, 1.0);
    d.bl = (float)luaL_optnumber(l, base + 2, 1.0);
    d.al = (float)luaL_optnumber(l, base + 3, 1.0);
}

static int l_line(lua_State* l) {
    DrawCmd d{}; d.type = 0;
    d.a = (float)luaL_checknumber(l,1); d.b = (float)luaL_checknumber(l,2);
    d.c = (float)luaL_checknumber(l,3); d.d = (float)luaL_checknumber(l,4);
    d.thick = (float)luaL_optnumber(l, 9, 1.0);
    pushColor(l, d, 5);
    s_draw.push_back(d); return 0;
}

static int l_rect(lua_State* l) {
    DrawCmd d{}; d.type = 1;
    d.a = (float)luaL_checknumber(l,1); d.b = (float)luaL_checknumber(l,2);
    d.c = (float)luaL_checknumber(l,3); d.d = (float)luaL_checknumber(l,4);
    d.thick = (float)luaL_optnumber(l, 9, 1.0);
    pushColor(l, d, 5);
    s_draw.push_back(d); return 0;
}

static int l_text(lua_State* l) {
    DrawCmd d{}; d.type = 2;
    d.a = (float)luaL_checknumber(l,1); d.b = (float)luaL_checknumber(l,2);
    d.text = luaL_checkstring(l, 3);
    pushColor(l, d, 4);
    s_draw.push_back(d); return 0;
}

static int l_circle(lua_State* l) {
    DrawCmd d{}; d.type = 3;
    d.a = (float)luaL_checknumber(l,1); d.b = (float)luaL_checknumber(l,2);
    d.c = (float)luaL_checknumber(l,3);
    d.thick = (float)luaL_optnumber(l, 8, 1.0);
    pushColor(l, d, 4);
    s_draw.push_back(d); return 0;
}

static int l_log(lua_State* l) {
    int n = lua_gettop(l);
    std::string out;
    for (int i = 1; i <= n; i++) {
        if (i > 1) out += "  ";
        size_t len = 0;
        const char* s = luaL_tolstring(l, i, &len);
        out += s ? s : "nil";
        lua_pop(l, 1);
    }
    Log(out);
    return 0;
}

static const luaL_Reg kFuncs[] = {
    {"base", l_base}, {"local_player", l_local}, {"screen_w", l_sw}, {"screen_h", l_sh},
    {"read8", l_read8}, {"read16", l_read16}, {"read32", l_read32},
    {"read64", l_read64}, {"readf", l_readf}, {"readd", l_readd},
    {"write8", l_write8}, {"write32", l_write32}, {"writef", l_writef}, {"writed", l_writed},
    {"world_to_screen", l_w2s}, {"players", l_players},
    {"draw_line", l_line}, {"draw_rect", l_rect}, {"draw_text", l_text}, {"draw_circle", l_circle},
    {"log", l_log},
    {nullptr, nullptr}
};

static void OnError(lua_State* l) {
    const char* msg = lua_tostring(l, -1);
    Log(std::string("[error] ") + (msg ? msg : "unknown"));
    lua_pop(l, 1);
}

bool Init() {
    L = luaL_newstate();
    if (!L) return false;
    luaL_openlibs(L);
    luaL_newlib(L, kFuncs);
    lua_setglobal(L, "cheat");
    Log("Lua 5.4 ready - cheat.* namespace loaded");
    return true;
}

void Shutdown() { if (L) { lua_close(L); L = nullptr; } }

void RunString(const std::string& code) {
    if (!L) return;
    if (luaL_loadstring(L, code.c_str()) != LUA_OK) { OnError(L); return; }
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) { OnError(L); return; }
    Log("> ok");
}

void RunFile(const std::string& path) {
    if (!L) return;
    if (luaL_loadfile(L, path.c_str()) != LUA_OK) { OnError(L); return; }
    if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) { OnError(L); return; }
    Log("> ran " + path);
}

std::vector<std::string> Snapshot() { return s_log; }
void ClearLog() { s_log.clear(); }
void ClearDraw() { s_draw.clear(); }

std::vector<DrawCmd> ConsumeDraw() {
    auto out = s_draw;
    s_draw.clear();
    return out;
}

} // namespace lua