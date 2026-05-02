#pragma once

#include <string>
#include <vector>

struct CommandParams {
    std::string raw;
    std::string cmd;
    std::string flags;
    std::vector<std::string> args;
    bool sudo = false;
};
