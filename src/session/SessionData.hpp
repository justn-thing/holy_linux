#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace SData {
    extern std::string username;
    extern bool root;
    extern std::vector<std::string> cmdHistory;

    namespace RAM {
        extern std::filesystem::path py;
        extern std::filesystem::path cpp;
        extern std::filesystem::path exe;
    }

    namespace ROM {
        extern std::filesystem::path fileSystem;
    }
}
