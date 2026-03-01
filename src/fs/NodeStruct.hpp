#pragma once

struct Metadata {
    bool sudo = false;
    std::string misc;
};

struct Node {
    std::string type;
    std::string name;
    std::string value;
    Metadata metadata;
    Node* parent;
    std::vector<std::unique_ptr<Node>> children;

    Node(std::string type, std::string name, std::string value = "", Metadata metadata = {false, ""}, Node* parent = nullptr)
    : type(std::move(type)), name(std::move(name)), value(std::move(value)), metadata(std::move(metadata)), parent(parent) {}
};