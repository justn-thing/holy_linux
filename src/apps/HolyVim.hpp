#pragma once

#include <string>
#include <vector>

#include "../fs/NodeStruct.hpp"

void displayIndex(const size_t& index);
size_t displayContent(const std::vector<std::string>& output);
void HolyVim(Node* target, const std::string& cmd);
