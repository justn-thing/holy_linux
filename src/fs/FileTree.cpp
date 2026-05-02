#include "FileTree.hpp"

#include <iostream>
#include <sstream>
#include <vector>

#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

namespace FS {
    std::unique_ptr<Node> unique_ptr_root = std::make_unique<Node>("dir", "root", "");
    Node* root = unique_ptr_root.get();
    Node* current = root;
}

Node* GetChild(const Node* parent, const std::string& name, const std::string& type) {
    for (const std::unique_ptr<Node>& child : parent->children) {
        if (child->name == name && type == child->type) {
            return child.get();
        }
    }

    return nullptr;
}

std::string GetPath(const Node* current) {
    std::string path;
    const Node* temp = current;

    while (temp) {
        path.insert(0, "/" + temp->name);
        temp = temp->parent;
    }

    return path;
}

std::string GetCosmeticPath(const Node* current) {
    std::string path = GetPath(current).substr(5);
    path = path.empty() ? "/" : path;

    if (path.starts_with("/home/" + SData::username))
        path = "~" + path.substr(6 + SData::username.length());

    return path;
}

Node* GetAbsolute(const std::string& arg, Node* current) {
    Node* temp = nullptr;
    std::vector<std::string> parts = split(arg, '/');

    if (arg.starts_with('/'))
        temp = FS::root;
    else if (arg == "~" || arg.starts_with("~/")) {
        const Node* home = GetChild(FS::root, "home", "dir");
        if (!home)
            return nullptr;
        Node* user = GetChild(home, SData::username, "dir");
        if (!user)
            return nullptr;

        parts.erase(parts.begin());
        temp = user;
    } else
        temp = current;

    if (parts.empty())
        return temp;

    for (const std::string& part : parts) {
        if (part == ".")
            continue;
        if (part == "..") {
            if (temp->parent)
                temp = temp->parent;
            continue;
        }
        const size_t index = part.rfind('.');
        std::string name = part;
        std::string type = "dir";
        if (index != std::string::npos && part.size() > index + 1) {
            name = part.substr(0, index);
            type = part.substr(index + 1);
        }

        if (Node* child = GetChild(temp, name, type))
            temp = child;
        else
            return nullptr;
    }

    return temp;
}

bool IsAncestor(const Node* parent, const Node* child) {
    const Node* temp = child;
    while (true) {
        if (temp == parent) {
            return true;
        }
        if (temp == FS::root) {
            return false;
        }
        temp = temp->parent;
    }
}

Node* NewChild(Node* parent, const std::string& name, const std::string& type, const bool protectedFile,
               const std::string& misc) {
    if (GetChild(parent, name, type))
        return nullptr;

    const Metadata metadata = {.sudo=protectedFile, .misc=misc};
    parent->children.push_back(std::make_unique<Node>(type, name, "", metadata, parent));

    return parent->children.back().get();
}

char RemoveNode(const Node* node, const bool recursive) {
    if (!node->children.empty() && !recursive) {
        alert(msg::dir_not_empty, stx::red);
        return 2;
    }

    for (auto iter = node->parent->children.begin(); iter != node->parent->children.end(); ++iter) {
        if (iter->get() == node) {
            node->parent->children.erase(iter);
            return 1;
        }
    }

    return 0;
}

short RemoveChild(const Node* parent, const std::string& name, const std::string& type, const bool recursive) {
    if (const Node* node = GetChild(parent, name, type)) {
        return RemoveNode(node, recursive);
    }

    return 0;
}

void DisplayDir(const Node* parent) {
    if (parent->children.empty()) {
        alert(msg::dir_empty, stx::red);
        return;
    }

    std::ostringstream buffer;
    for (const std::unique_ptr<Node>& child : parent->children) {
        if (child->type == "dir") {
            buffer << child->name << " ";
        } else {
            buffer << child->name << "." << child->type << " ";
        }
    }
    buffer << "\n";
    std::cout << buffer.str();
}

void LockNode(Node* node, const bool lock) {
    node->metadata.sudo = lock;

    if (node->type == "dir") {
        for (const std::unique_ptr<Node>& child : node->children) {
            LockNode(child.get(), lock);
        }
    }
}
