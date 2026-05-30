#include "Execution.hpp"

#include <chrono>
#include <filesystem>
#include <string>
#include <utility>

#include "../apps/HolyVim.hpp"
#include "../cmd/CmdParser.hpp"
#include "../cmd/Cmds.hpp"
#include "../util/ReturnCodes.hpp"
#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Syntax.hpp"

int TryExecutePackage(CmdParams& param) {
    const Node* bin = GetChild(FS::root, "bin", "dir");
    if (!bin)
        bin = NewChild(FS::root, "bin", "dir");

    const std::string exec = param.sudo ? "sudo exec /bin/" : "exec /bin/";

    if (GetChild(bin, param.cmd, "exe")) {
        CmdParams line = ParseCommandLine(exec + param.cmd + ".exe");
        return ExecuteCmdLine(line);
    } if (GetChild(bin, param.cmd, "cmd")) {
        CmdParams line = ParseCommandLine(exec + param.cmd + ".cmd");
        return ExecuteCmdLine(line);
    } if (GetChild(bin, param.cmd, "py")) {
        CmdParams line = ParseCommandLine(exec + param.cmd + ".py");
        return ExecuteCmdLine(line);
    }

    alert(msg::unknown_cmd, stx::yellow);
    return 1;
}

int ExecuteCmd(CmdParams& param) {
    if (param.cmd.empty() || param.cmd == "//")
        return 0;

    if (const auto it = Commands.find(param.cmd); it != Commands.end())
        return it->second(param);

    return TryExecutePackage(param);
}

int ExecuteCmdLine(CmdParams& param) {
    if (param.redirectMode == RedirectMode::Overwrite || param.redirectMode == RedirectMode::Append) {
        if (!param.redirectTarget) {
            alert(msg::file_not_found, stx::red);
            return 1;
        }

        if (param.redirectTarget->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 1;
        }

        if (param.redirectTarget->type == "dir") {
            alert(msg::invalid_file_type, stx::yellow);
            return 1;
        }

        SData::redirecting = true;
        SData::redirectTarget = param.redirectTarget;

        if (param.redirectMode == RedirectMode::Overwrite)
            SData::redirectTarget->value.clear();
    } else {
        SData::redirecting = false;
        SData::redirectTarget = nullptr;
    }

    const int returnCode = ExecuteCmd(param);
    if (returnCode == Poweroff || returnCode == Reboot) {
        SData::redirecting = false;
        SData::redirectTarget = nullptr;
        return returnCode;
    }

    if (((param.redirectMode == RedirectMode::And && returnCode == 0) ||
        (param.redirectMode == RedirectMode::Or && returnCode == 1)) &&
        param.redirectCmd) {
        const int otherReturnCode = ExecuteCmdLine(*param.redirectCmd);

        SData::redirecting = false;
        SData::redirectTarget = nullptr;
        return otherReturnCode;
    }

    SData::redirecting = false;
    SData::redirectTarget = nullptr;

    return returnCode;
}
