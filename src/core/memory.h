#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <cstdint>

namespace mem {

inline HANDLE    g_handle = nullptr;
inline DWORD     g_pid    = 0;
inline uintptr_t g_base   = 0;

inline uintptr_t ModuleBase(DWORD pid, const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32W me{}; me.dwSize = sizeof(me);
    uintptr_t out = 0;
    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(me.szModule, name) == 0) { out = (uintptr_t)me.modBaseAddr; break; }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return out;
}

inline DWORD FindProcess(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe{}; pe.dwSize = sizeof(pe);
    DWORD out = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, name) == 0) { out = pe.th32ProcessID; break; }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return out;
}

inline bool Attach(const wchar_t* procName) {
    if (g_handle) { CloseHandle(g_handle); g_handle = nullptr; }

    DWORD pid = FindProcess(procName);
    if (!pid) return false;

    HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                           PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!h) h = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return false;

    g_handle = h;
    g_pid    = pid;
    g_base   = ModuleBase(pid, procName);
    return g_base != 0;
}

inline void Detach() {
    if (g_handle) CloseHandle(g_handle);
    g_handle = nullptr; g_pid = 0; g_base = 0;
}

inline bool Alive() {
    if (!g_handle) return false;
    DWORD code = 0;
    if (!GetExitCodeProcess(g_handle, &code)) return false;
    return code == STILL_ACTIVE;
}

inline bool ReadRaw(uintptr_t addr, void* buf, size_t size) {
    if (!g_handle || !addr) return false;
    SIZE_T read = 0;
    return ReadProcessMemory(g_handle, (LPCVOID)addr, buf, size, &read) && read == size;
}

template <typename T>
inline T Read(uintptr_t addr) {
    T buf{};
    ReadRaw(addr, &buf, sizeof(T));
    return buf;
}

template <typename T>
inline bool Write(uintptr_t addr, const T& val) {
    if (!g_handle || !addr) return false;
    SIZE_T written = 0;
    return WriteProcessMemory(g_handle, (LPVOID)addr, &val, sizeof(T), &written) != 0;
}

inline uintptr_t ReadPtr(uintptr_t addr) { return Read<uintptr_t>(addr); }

inline std::string ReadString(uintptr_t addr, size_t maxLen = 32) {
    if (!addr) return {};
    char buf[64]{};
    size_t n = maxLen < 63 ? maxLen : 63;
    if (!ReadRaw(addr, buf, n)) return {};
    buf[n] = 0;
    return std::string(buf);
}

} // namespace mem