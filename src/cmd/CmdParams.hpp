#pragma once

#include <memory>
#include <string>
#include <vector>

enum class RedirectMode {
    None,
    And,
    Or,
    Overwrite,
    Append
};

struct CmdParams {
    std::string cmd;
    std::string shortFlags;
    std::vector<std::string> longFlags;
    std::vector<std::string> args;
    bool sudo = false;

    RedirectMode redirectMode = RedirectMode::None;
    std::unique_ptr<CmdParams> redirectCmd;
    std::string redirectTarget;
};
