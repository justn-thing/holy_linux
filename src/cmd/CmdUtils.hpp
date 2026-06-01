#pragma once

#include <array>

#include "../cmd/CmdParams.hpp"
#include "../fs/NodeStruct.hpp"

constexpr std::array<std::string_view, 4> textFileTypes = {
    "txt", "cmd", "py", "cpp"
};

constexpr std::array<std::string_view, 3> execFileTypes = {
    "exe", "cmd", "py"
};

bool NotSudo(const CmdParams& param);
bool NoArgs(const CmdParams& param);
bool NotEnoughArgs(const CmdParams& param, size_t minAmount);
bool IsTextFileType(const std::string& fileType);
bool IsExecutableFileType(const std::string& fileType);
bool IsOverflowingStack(const Node* file);
void AddToStack(const Node* file);
void RemoveFromStack(const Node* file);
