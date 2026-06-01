#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../fs/NodeStruct.hpp"

class CurrentUser {
public:
    std::string name = "//root//";
    bool root = true;
    std::vector<std::string> cmdHistory;
};

namespace SData {
    extern CurrentUser user;

    extern bool redirecting;
    extern Node* redirectTarget;

    extern std::vector<const Node*> executionStack;

    namespace ExternFS {
        extern std::filesystem::path selfParentPath;

        namespace RAM {
            extern std::filesystem::path py;
            extern std::filesystem::path cpp;
            extern std::filesystem::path exe;
        }

        namespace ROM {
            extern std::filesystem::path fileSystem;
        }

        namespace Export {
            extern std::filesystem::path selfDir;
        }
    }
}
