#pragma once

#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "../fs/FileTree.hpp"
#include "../fs/NodeStruct.hpp"

inline std::string SerializeNode(const Node* node);

inline std::string SerializeDir(const Node* dir) {
    std::string value;
    for (const std::unique_ptr<Node>& child : dir->children) {
        value += SerializeNode(child.get());
    }
    return value;
}

inline std::string SerializeNode(const Node* node) {
    std::string payload;

    if (node->type == "dir") {
        payload = SerializeDir(node);
    } else {
        payload = node->value;
    }

    const std::string& misc = node->metadata.misc;

    std::string out;
    out += node->name + "." + node->type + " ";
    out += std::to_string(payload.size()) + " ";
    out += std::to_string(node->metadata.sudo ? 1 : 0) + " ";
    out += std::to_string(misc.size()) + "\n";
    out += misc;
    out += payload;

    return out;
}

inline bool SaveFileSystem() {
    std::ofstream out("rom/fileSystem.txt", std::ios::binary);
    if (!out.is_open()) return false;

    for (const std::unique_ptr<Node>& child : FS::root->children) {
        out << SerializeNode(child.get());
    }
    return true;
}

inline bool LoadNode(Node* parent, std::istream& in);

inline void LoadDir(Node* parent, std::istream& in, const size_t limit) {
    std::string buf(limit, '\0');

    if (limit > static_cast<size_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::runtime_error("Loading filesystem failed; directory size too large");
    }

    in.read(buf.data(), static_cast<std::streamsize>(limit));
    if (std::cmp_not_equal(in.gcount(), limit)) {
        throw std::runtime_error("Loading filesystem failed; unexpected EOF in directory");
    }

    std::istringstream sub(buf);
    while (LoadNode(parent, sub)) {}
}

inline bool LoadNode(Node* parent, std::istream& in)
{
    std::string header;
    if (!getline(in, header))
        return false; // clean EOF

    if (header.empty())
        return false; // ignore trailing newline safely

    std::string nameType;
    size_t payloadSize;
    unsigned char sudoFlag;
    size_t miscSize;

    {
        if (std::stringstream ss(header);
            !(ss >> nameType >> payloadSize >> sudoFlag >> miscSize))
            throw std::runtime_error("Loading filesystem failed; corrupt header");
    }

    const size_t dot = nameType.rfind('.');
    if (dot == std::string::npos || dot == 0 || dot == nameType.size() - 1)
        throw std::runtime_error("Loading filesystem failed; corrupt name/type");

    const std::string name = nameType.substr(0, dot);
    const std::string type = nameType.substr(dot + 1);

    Metadata md;
    md.sudo = (sudoFlag != 0);

    // read misc metadata
    md.misc.resize(miscSize);
    in.read(md.misc.data(), static_cast<std::streamsize>(miscSize));
    if (std::cmp_not_equal(in.gcount(), miscSize))
        throw std::runtime_error("Loading filesystem failed; unexpected EOF in metadata");

    Node* node = NewChild(parent, name, type, md.sudo, md.misc);

    if (type == "dir") {
        LoadDir(node, in, payloadSize);
    } else {
        node->value.resize(payloadSize);
        in.read(node->value.data(), static_cast<std::streamsize>(payloadSize));

        if (std::cmp_not_equal(in.gcount(), payloadSize))
            throw std::runtime_error("Loading filesystem failed; unexpected EOF in file");
    }

    return true;
}

inline bool LoadFileSystem() {
    std::ifstream fs("rom/fileSystem.txt", std::ios::binary);
    if (!fs.is_open()) return false;

    while (LoadNode(FS::root, fs)) {}
    return true;
}
