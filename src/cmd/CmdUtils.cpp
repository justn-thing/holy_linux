#include "CmdUtils.hpp"

#include <algorithm>
#include <array>
#include <string_view>

#include "../cmd/CmdParams.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Syntax.hpp"

bool NotSudo(const CmdParams& param) {
    if (!param.sudo) {
        alert(msg::not_sudo, stx::yellow);
        return true;
    }

    return false;
}

bool NoArgs(const CmdParams& param) {
    if (param.args.empty()) {
        alert(msg::arg_missing, stx::yellow);
        return true;
    }

    return false;
}

bool NotEnoughArgs(const CmdParams& param, const size_t minAmount) {
    if (param.args.size() < minAmount) {
        alert(msg::arg_missing, stx::yellow);
        return true;
    }

    return false;
}

bool IsTextFileType(const std::string& fileType) {
    constexpr std::array<std::string_view, 4> textTypes = {
        "txt", "cmd", "py", "cpp"
    };

    return std::ranges::contains(textTypes, fileType);
}
