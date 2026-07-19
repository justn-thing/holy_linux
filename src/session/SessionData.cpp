#include "SessionData.hpp"

#include <filesystem>

#include "../fs/Node.hpp"

namespace SData {
    CurrentUser user;

    bool redirecting = false;
    Node* redirectTarget = nullptr;

    std::vector<const Node*> executionStack {};

    namespace ExternFS {
        std::filesystem::path selfParentPath;

        namespace RAM {
            std::filesystem::path py;
            std::filesystem::path cpp;
            std::filesystem::path exe;
        }

        namespace ROM {
            std::filesystem::path fileSystem;
        }

        namespace Export {
            std::filesystem::path selfDir;
        }
    }
}
