#pragma once

#include <memory>
#include <string>
#include <vector>

struct Metadata {
    bool sudo = false;
    std::string misc;
};

class Node {
    void AddTreeNodeName(std::string& buffer) const;
    void PrintTreeChildren(const std::string& prefix) const;

public:
    std::string _type;
    std::string _name;
    std::string _value;
    Metadata _metadata;
    Node* _parent;
    std::vector<std::unique_ptr<Node>> _children;

    Node(std::string type, std::string name, std::string value = "", Metadata metadata = {false, ""},
         Node* parent = nullptr);

    Node* GetChild(const std::string& name, const std::string& type) const noexcept;
    std::string GetPath() const;
    std::string GetCosmeticPath() const;
    Node* GetAbsolute(const std::string& arg);
    bool IsAncestorOf(const Node* descendant) const noexcept;
    Node* NewChild(const std::string& name, const std::string& type, bool protectedFile = false, const std::string& misc = "");
    int Remove(bool recursive = false) const;
    int RemoveChild(const std::string& name, const std::string& type, bool recursive = false) const;
    void SetLock(bool lock, bool recursive = false) noexcept;
    size_t GetFileSize() const;
    bool Copy(Node* dest, bool sudo) const;
    bool ContainsLockedNode() const noexcept;
    bool Move(Node* dest, bool sudo) const;
    void FindDescendants(const std::string& name, const std::string& type, std::vector<Node*>& buffer, bool anyType = false);
    void DisplayDir() const;
    void PrintTree(int indent = 0) const;
};

Node* GetAbsolute(const std::string& arg);
