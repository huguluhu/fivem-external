#include "config.h"
#include "core/globals.h"
#include "ui/notify.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

namespace config {

namespace fs = std::filesystem;

std::string Dir() {
    char path[MAX_PATH]{};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    fs::path p(path);
    fs::path d = p.parent_path() / "configs";
    std::error_code ec;
    fs::create_directories(d, ec);
    return d.string();
}

static fs::path File(const std::string& name) {
    std::string n = name.empty() ? "default" : name;
    if (n.find(".ini") == std::string::npos) n += ".ini";
    return fs::path(Dir()) / n;
}

struct Writer {
    std::ofstream f;
    explicit Writer(const fs::path& p) : f(p) {}
    void Section(const char* s) { f << "\n[" << s << "]\n"; }
    void Val(const char* k, bool v)  { f << k << "=" << (v ? 1 : 0) << "\n"; }
    void Val(const char* k, int v)   { f << k << "=" << v << "\n"; }
    void Val(const char* k, float v) { f << k << "=" << v << "\n"; }
    void Val(const char* k, const RGBA& c) {
        f << k << "=" << c.r << "," << c.g << "," << c.b << "," << c.a << "\n";
    }
};

struct Reader {
    std::vector<std::pair<std::string, std::string>> kv;
    explicit Reader(const fs::path& p) {
        std::ifstream f(p);
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '[' || line[0] == ';' || line[0] == '#') continue;
            auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            kv.emplace_back(line.substr(0, eq), line.substr(eq + 1));
        }
    }
    const std::string* Find(const char* k) const {
        for (auto& e : kv) if (e.first == k) return &e.second;
        return nullptr;
    }
    void Val(const char* k, bool& v) const { auto s = Find(k); if (s) v = (*s == "1"); }
    void Val(const char* k, int& v)  const { auto s = Find(k); if (s) v = atoi(s->c_str()); }
    void Val(const char* k, float& v)const { auto s = Find(k); if (s) v = (float)atof(s->c_str()); }
    void Val(const char* k, RGBA& c) const {
        auto s = Find(k);
        if (!s) return;
        std::stringstream ss(*s);
        std::string part;
        float vals[4]{}; int i = 0;
        while (std::getline(ss, part, ',') && i < 4) vals[i++] = (float)atof(part.c_str());
        if (i == 4) c = RGBA(vals[0], vals[1], vals[2], vals[3]);
    }
};

bool Save(const std::string& name) {
    Writer w(File(name));
    if (!w.f) return false;

    w.Section("aimbot");
#define X(t,n,d) w.Val(#n, cfg.aim.n);
    AIM_FIELDS(X)
#undef X

    w.Section("esp");
#define X(t,n,d) w.Val(#n, cfg.esp.n);
    ESP_FIELDS(X)
#undef X

    w.Section("vehicle");
#define X(t,n,d) w.Val(#n, cfg.veh.n);
    VEH_FIELDS(X)
#undef X

    w.Section("misc");
#define X(t,n,d) w.Val(#n, cfg.misc.n);
    MISC_FIELDS(X)
#undef X

    return true;
}

bool Load(const std::string& name) {
    fs::path p = File(name);
    if (!fs::exists(p)) return false;
    Reader r(p);

#define X(t,n,d) r.Val(#n, cfg.aim.n);
    AIM_FIELDS(X)
#undef X
#define X(t,n,d) r.Val(#n, cfg.esp.n);
    ESP_FIELDS(X)
#undef X
#define X(t,n,d) r.Val(#n, cfg.veh.n);
    VEH_FIELDS(X)
#undef X
#define X(t,n,d) r.Val(#n, cfg.misc.n);
    MISC_FIELDS(X)
#undef X
    return true;
}

void Reset() { cfg = Config{}; }

} // namespace config