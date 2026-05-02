#include "NodeStruct.hpp"

#include <utility>

Node::Node(std::string type, std::string name, std::string value, Metadata metadata, Node* parent)
    : type(std::move(type)),
      name(std::move(name)),
      value(std::move(value)),
      metadata(std::move(metadata)),
      parent(parent) {}
