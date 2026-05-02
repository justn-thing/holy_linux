#pragma once

#include <memory>
#include <string>
#include <vector>

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

    Node(std::string type, std::string name, std::string value = "", Metadata metadata = {false, ""},
         Node* parent = nullptr);
};
