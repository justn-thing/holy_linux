#include "SessionData.hpp"

#include <filesystem>

namespace SData {
    std::string username = "//root//";
    bool root = true;
    std::vector<std::string> cmdHistory;

    namespace RAM {
        std::filesystem::path py;
        std::filesystem::path cpp;
        std::filesystem::path exe;
    }

    namespace ROM {
        std::filesystem::path fileSystem;
    }
}
