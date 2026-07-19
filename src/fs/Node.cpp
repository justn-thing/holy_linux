#include "Node.hpp"

#include <algorithm>
#include <utility>
#include <vector>

#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

Node::Node(std::string type, std::string name, std::string value, Metadata metadata, Node* parent)
    : _type(std::move(type)), _name(std::move(name)), _value(std::move(value)), _metadata(std::move(metadata)), _parent(parent) {}

Node* Node::GetChild(const std::string& name, const std::string& type) const noexcept {
    for (const std::unique_ptr<Node>& child : _children) {
        if (child->_name == name && type == child->_type) {
            return child.get();
        }
    }

    return nullptr;
}

std::string Node::GetPath() const {
    std::string path;
    const Node* temp = this;

    while (temp && temp != FS::root) {
        path.insert(0, "/" + temp->_name);
        temp = temp->_parent;
    }

    return path.empty() ? "/" : path;
}

std::string Node::GetCosmeticPath() const {
    std::string path = GetPath();

    if (path == "/home/" + SData::user.name || path.starts_with("/home/" + SData::user.name + '/')) {
        path = "~" + path.substr(6 + SData::user.name.length());
    }

    return path;
}

Node* Node::GetAbsolute(const std::string& arg) {
    Node* temp = nullptr;
    std::vector<std::string> parts = split(arg, '/');

    if (arg.starts_with('/')) {
        temp = FS::root;
    } else if (arg == "~" || arg.starts_with("~/")) {
        const Node* home = FS::root->GetChild("home", "dir");
        if (!home) {
            return nullptr;
        }
        Node* user = home->GetChild(SData::user.name, "dir");
        if (!user) {
            return nullptr;
        }

        parts.erase(parts.begin());
        temp = user;
    } else {
        temp = this;
    }

    if (parts.empty()) {
        return temp;
    }

    for (const std::string& part : parts) {
        if (part == ".") {
            continue;
        }
        if (part == "..") {
            if (temp->_parent) {
                temp = temp->_parent;
            }
            continue;
        }
        const size_t index = part.rfind('.');
        std::string name = part;
        std::string type = "dir";
        if (index != std::string::npos && part.size() > index + 1) {
            name = part.substr(0, index);
            type = part.substr(index + 1);
        }

        if (Node* child = temp->GetChild(name, type)) {
            temp = child;
        } else {
            return nullptr;
        }
    }

    return temp;
}

Node* GetAbsolute(const std::string& arg) {
    return FS::current->GetAbsolute(arg);
}

bool Node::IsAncestorOf(const Node* descendant) const noexcept {
    for (const Node* temp = descendant; temp; temp = temp->_parent) {
        if (temp == this) {
            return true;
        }
    }

    return false;
}

Node* Node::NewChild(const std::string& name, const std::string& type, const bool protectedFile, const std::string& misc) {
    if (GetChild(name, type)) {
        return nullptr;
    }

    const Metadata metadata = {.sudo=protectedFile, .misc=misc};
    _children.emplace_back(std::make_unique<Node>(type, name, "", metadata, this));

    return _children.back().get();
}

int Node::Remove(const bool recursive) const {
    if (!_parent) {
        return 0;
    }

    if (!_children.empty() && !recursive) {
        alert(msg::dir_not_empty, stx::red);
        return 2;
    }

    for (auto iter = _parent->_children.begin(); iter != _parent->_children.end(); ++iter) {
        if (iter->get() == this) {
            _parent->_children.erase(iter);
            return 1;
        }
    }

    return 0;
}

int Node::RemoveChild(const std::string& name, const std::string& type, const bool recursive) const {
    if (const Node* node = GetChild(name, type)) {
        return node->Remove(recursive);
    }

    return 0;
}

void Node::SetLock(const bool lock, const bool recursive) noexcept {
    _metadata.sudo = lock;

    if (recursive) {
        for (const std::unique_ptr<Node>& child : _children) {
            child->SetLock(lock, true);
        }
    }
}

size_t Node::GetFileSize() const {
    size_t payloadSize = 0;
    if (_type == "dir") {
        for (const std::unique_ptr<Node>& child : _children) {
            payloadSize += child->GetFileSize();
        }
    } else {
        payloadSize = _value.length();
    }

    if (this == FS::root) {
        return payloadSize;
    }

    const size_t miscSize = _metadata.misc.length();
    const size_t headerSize = _name.length()
                              + 1 // "\0"
                              + _type.length()
                              + 1 // "\0"
                              + std::to_string(payloadSize).length()
                              + 1 // "\0"
                              + 1 // sudo flag
                              + 1 // "\0"
                              + std::to_string(miscSize).length()
                              + 1; // "\0"

    return headerSize + miscSize + payloadSize;
}

bool Node::Copy(Node* dest, const bool sudo) const {
    if (!dest || dest->_type != "dir" || IsAncestorOf(dest)) {
        return false;
    }

    if ((_metadata.sudo || dest->_metadata.sudo) && !sudo) {
        alert(msg::not_sudo, stx::yellow);
        return false;
    }

    Node* newNode = dest->NewChild(_name, _type, _metadata.sudo, _metadata.misc);
    if (newNode == nullptr) {
        alert(msg::file_alr_exists, stx::red);
        return false;
    }
    newNode->_value = _value;

    return std::ranges::all_of(_children, [newNode, sudo] (const std::unique_ptr<Node>& child) {
        return child->Copy(newNode, sudo);
    });
}

bool Node::ContainsLockedNode() const noexcept {
    return _metadata.sudo || std::ranges::any_of(_children, []
        (const std::unique_ptr<Node>& child) {
        return child->ContainsLockedNode();
    });
}

bool Node::Move(Node* dest, const bool sudo) const {
    if (!dest || this == FS::root || IsAncestorOf(dest)) {
        return false;
    }

    if ((ContainsLockedNode() || dest->_metadata.sudo) && !sudo) {
        alert(msg::not_sudo, stx::yellow);
        return false;
    }

    if (dest->GetChild(_name, _type)) {
        alert(msg::file_alr_exists, stx::yellow);
        return false;
    }

    const auto iter = std::ranges::find_if(_parent->_children, [this]
        (const std::unique_ptr<Node>& child) {
        return child.get() == this;
    });

    if (iter == _parent->_children.end()) {
        return false;
    }

    std::unique_ptr<Node> movedNode = std::move(*iter);
    _parent->_children.erase(iter);

    movedNode->_parent = dest;
    dest->_children.emplace_back(std::move(movedNode));

    return true;
}

void Node::FindDescendants(const std::string& name, const std::string& type, std::vector<Node*>& buffer, const bool anyType) {
    if (_name == name && (anyType || _type == type)) {
        buffer.emplace_back(this);
    }

    for (const std::unique_ptr<Node>& child : _children) {
        child->FindDescendants(name, type, buffer, anyType);
    }
}

namespace {
    void AddFileNameSeparator(std::string& buffer) {
        buffer += stx::yellow;
        buffer += "\"";
        buffer += stx::reset;
    }
}

void Node::AddTreeNodeName(std::string& buffer) const {
    const bool quoted = _name.contains(' ') || _type.contains(' ');

    if (quoted) AddFileNameSeparator(buffer);

    buffer += _name;
    if (_type != "dir") {
        buffer += '.';
        buffer += _type;
    }

    if (quoted) AddFileNameSeparator(buffer);
}

void Node::DisplayDir() const {
    if (_children.empty()) {
        alert(msg::dir_empty, stx::red);
        return;
    }

    std::string buffer;
    for (const std::unique_ptr<Node>& child : _children) {
        const bool containsSpace = child->_name.contains(' ') || child->_type.contains(' ');
        if (containsSpace) AddFileNameSeparator(buffer);

        buffer += child->_name;
        if (child->_type != "dir") {
            buffer += '.' + child->_type;
        }

        if (containsSpace) AddFileNameSeparator(buffer);

        buffer += '\n';
    }
    msg::cout << buffer;
}

void Node::PrintTreeChildren(const std::string& prefix) const {
    for (size_t i = 0; i < _children.size(); ++i) {
        const Node* child = _children[i].get();
        const bool isLast = i == _children.size() - 1;

        std::string buffer = prefix;
        buffer += isLast ? "└── " : "├── ";
        child->AddTreeNodeName(buffer);
        buffer += '\n';
        msg::cout << buffer;

        child->PrintTreeChildren(prefix + (isLast ? "    " : "│   "));
    }
}

void Node::PrintTree(const int indent) const {
    for (int i = 0; i < indent; ++i) {
        msg::cout << ' ';
    }

    msg::cout << GetPath() << '\n';
    PrintTreeChildren(std::string(indent, ' '));
}
