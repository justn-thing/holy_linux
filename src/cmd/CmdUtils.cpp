#include "CmdUtils.hpp"

#include <algorithm>
#include <array>

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
    return std::ranges::contains(textFileTypes, fileType);
}

bool IsExecutableFileType(const std::string& fileType) {
    return std::ranges::contains(execFileTypes, fileType);
}

bool IsOverflowingStack(const Node* file) {
    if (std::ranges::count(SData::executionStack, file) > 63) {
        alert(msg::stack_overflow, stx::red);
        return true;
    }

    return false;
}

void AddToStack(const Node* file) {
    SData::executionStack.emplace_back(file);
}

void RemoveFromStack(const Node* file) {
    if (const auto iter = std::ranges::find(SData::executionStack, file);
        iter != SData::executionStack.end()) {
        SData::executionStack.erase(iter);
    }
}
