#pragma once

#include <string>
#include <vector>

std::vector<std::string> split(const std::string& s, char delimiter, bool preserveEmpty = false,
                               bool preserveLast = false);
