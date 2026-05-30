#pragma once

#include <string_view>
#include <unordered_map>

#include "../cmd/CmdParams.hpp"

extern const std::unordered_map<std::string_view, int (*)(const CmdParams&)> Commands;
