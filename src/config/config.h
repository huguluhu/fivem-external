#pragma once
#include <string>

namespace config {
    std::string Dir();
    bool Save(const std::string& name);
    bool Load(const std::string& name);
    void Reset();
}