#pragma once

#include <istream>
#include <string>

#include "NodeStruct.hpp"

std::string SerializeNode(const Node* node);
std::string SerializeDir(const Node* dir);
bool SaveFileSystem();
bool LoadNode(Node* parent, std::istream& in);
void LoadDir(Node* parent, std::istream& in, size_t limit);
bool LoadFileSystem();
