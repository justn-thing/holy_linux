#pragma once

#include <memory>
#include <string>

#include "NodeStruct.hpp"

namespace FS {
    extern std::unique_ptr<Node> unique_ptr_root;
    extern Node* root;
    extern Node* current;
}

Node* GetChild(const Node* parent, const std::string& name, const std::string& type);
std::string GetPath(const Node* current);
std::string GetCosmeticPath(const Node* current = FS::current);
Node* GetAbsolute(const std::string& arg, Node* current = FS::current);
bool IsAncestor(const Node* parent, const Node* child);
Node* NewChild(Node* parent, const std::string& name, const std::string& type, bool protectedFile = false,
               const std::string& misc = "");
char RemoveNode(const Node* node, bool recursive = false);
short RemoveChild(const Node* parent, const std::string& name, const std::string& type, bool recursive = false);
void DisplayDir(const Node* parent);
void LockNode(Node* node, bool lock);
