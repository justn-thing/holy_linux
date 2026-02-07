#pragma once

#include <vector>
#include "Misc.h"

using namespace std;

struct Metadata {
    bool sudo = false;
    string misc;
};

struct Node {
    const string type;
    string name;
    string value;
    Metadata metadata;
    Node* parent;
    vector<unique_ptr<Node>> children;

    Node(string type, string name, string value = "", Metadata metadata = {false, ""}, Node* parent = nullptr)
    : type(std::move(type)), name(std::move(name)), value(std::move(value)), metadata(std::move(metadata)), parent(parent) {}
};

inline Node* newChild(Node* parent, const string& name, const string& type, const bool protectedFile = false,
    const string& misc = "") {
    Metadata metadata = {protectedFile, misc};
    parent->children.push_back(make_unique<Node>(type, name, "", metadata, parent));

    return parent->children.back().get();
}

inline Node* getChild(const Node* parent, const string& name, const string& type) {
    for (const unique_ptr<Node>& child : parent->children) {
        if (child->name == name && type == child->type) {
            return child.get();
        }
    }

    return nullptr;
}

inline string getPath(const Node* current) {
    string path;
    const Node* temp = current;

    while (temp) {
        path.insert(0, "/" + temp->name);
        temp = temp->parent;
    }

    return path;
}

inline short removeNode(const Node* node, const bool recursive = false) {
    if (!node->children.empty() && !recursive) {
        throwError(err::dir_not_empty, stx::red);
        return 2; // other
    }

    for (auto iter = node->parent->children.begin(); iter != node->parent->children.end(); ++iter) {
        if (iter->get() == node) {
            node->parent->children.erase(iter);
            return 1; // success
        }
    }

    return 0; // fail
}

inline short removeChild(const Node* parent, const string& name, const string& type, const bool recursive = false) {
    if (const Node* node = getChild(parent, name, type)) {
        return removeNode(node, recursive);
    }

    return 0; // fail
}

inline Node* getAbsolute(Node* root, const string& arg, const string& type, const bool startup = false) {
    if (arg == "/root" || arg == "/root/") {
        return root;
    }

    if (!arg.starts_with("/root/")) {
        throwError(err::invalid_abs_path, stx::yellow);
        return nullptr;
    }

    const string trimmed = arg.substr(6);
    const vector<string> parts = split(trimmed, '/');

    const Node* temp = root;
    for (auto i{0uz}; i + 1 < parts.size(); ++i) {
        if (const Node* child = getChild(temp, parts[i], "dir")) {
            temp = child;
        } else {
            throwError(err::_not_found, stx::red, startup, type);
            return nullptr;
        }
    }

    vector<string> last = split(parts.back(), '.');
    if (const size_t lastSize = last.size();
        lastSize < 2) {
        last.emplace_back("dir");
    } else if (lastSize > 2) {
        throwError(err::invalid_abs_path, stx::yellow);
        return nullptr;
    }

    if (Node* file = getChild(temp, last[0], last[1])) {
        return file;
    }

    throwError(err::_not_found, stx::red, startup, type);
    return nullptr;
}

inline void displayDir(const Node* parent) {
    if (parent->children.empty()) {
        throwError(err::dir_empty, stx::red);
        return;
    }

    ostringstream buffer;
    for (const unique_ptr<Node>& child : parent->children) {
        if (child->type == "dir") {
            buffer << child->name << " ";
        } else {
            buffer << child->name << "." << child->type << " ";
        }
    }
    buffer << "\n";
    cout << buffer.str();
}

inline bool isAncestor(const Node* root, const Node* parent, const Node* child) {
    const Node* temp = child;
    while (true) {
        if (temp == parent) {
            return true;
        }
        if (temp == root) {
            return false;
        }
        temp = temp->parent;
    }
}

inline void lockNode(Node* node, const bool lock) {
    node->metadata.sudo = lock;

    if (node->type == "dir") {
        for (unique_ptr<Node>& child : node->children) {
            lockNode(child.get(), lock);
        }
    }
}