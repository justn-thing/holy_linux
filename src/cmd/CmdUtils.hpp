#pragma once

#include "../cmd/CmdParams.hpp"

bool NotSudo(const CmdParams& param);
bool NoArgs(const CmdParams& param);
bool NotEnoughArgs(const CmdParams& param, size_t minAmount);
bool IsTextFileType(const std::string& fileType);
