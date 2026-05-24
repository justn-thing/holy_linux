#pragma once

#include <string>
#include <vector>

struct CommandParams {
    std::string raw;
    std::string cmd;
    std::string shortFlags;
    std::vector<std::string> longFlags;
    std::vector<std::string> args;
    bool sudo = false;
};
