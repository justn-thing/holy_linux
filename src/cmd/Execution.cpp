#include "Execution.hpp"

#include <chrono>
#include <filesystem>
#include <string>
#include <utility>

#include "../cmd/CmdParser.hpp"
#include "../cmd/Cmds.hpp"
#include "../cmd/CmdUtils.hpp"
#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/ReturnCodes.hpp"

namespace {
    int TryExecutePackage(const CmdParams& param) {
        const Node* bin = FS::root->GetChild("bin", "dir");
        if (!bin) {
            bin = FS::root->NewChild("bin", "dir");
        }

        const std::string exec = param.sudo ? "sudo exec /bin/" : "exec /bin/";

        for (const std::string_view& execFileType : execFileTypes) {
            if (bin->GetChild(param.cmd, execFileType.data())) {
                const CmdParams line = ParseCommandLine(exec + param.cmd + '.' + execFileType.data());
                return ExecuteCmdLine(line);
            }
        }

        alert(msg::unknown_cmd, stx::yellow);
        return 1;
    }

    int ExecuteCmd(const CmdParams& param) {
        if (param.cmd.empty() || param.cmd == "//") {
            return 0;
        }

        if (const auto it = Commands.find(param.cmd); it != Commands.end()) {
            return it->second(param);
        }

        return TryExecutePackage(param);
    }
}

int ExecuteCmdLine(const CmdParams& param) {
    if (param.redirectMode == RedirectMode::Overwrite || param.redirectMode == RedirectMode::Append) {
        Node* redirectTarget = GetAbsolute(param.redirectTarget);

        if (!redirectTarget) {
            alert(msg::file_not_found, stx::red);
            return 1;
        }

        if (redirectTarget->_metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 1;
        }

        if (redirectTarget->_type == "dir") {
            alert(msg::invalid_file_type, stx::yellow);
            return 1;
        }

        SData::redirecting = true;
        SData::redirectTarget = redirectTarget;

        if (param.redirectMode == RedirectMode::Overwrite) {
            SData::redirectTarget->_value.clear();
        }
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
