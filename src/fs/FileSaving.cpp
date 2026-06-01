#include "FileSaving.hpp"

#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"

std::string SerializeDir(const Node* dir) {
    std::string value;
    for (const std::unique_ptr<Node>& child : dir->children) {
        value += SerializeNode(child.get());
    }
    return value;
}

std::string SerializeNode(const Node* node) {
    std::string payload;

    if (node->type == "dir") {
        payload = SerializeDir(node);
    } else {
        payload = node->value;
    }

    const std::string& misc = node->metadata.misc;

    std::string out;
    out += node->name + '\0';
    out += node->type + '\0';
    out += std::to_string(payload.size()) + '\0';
    out += std::to_string(node->metadata.sudo ? 1 : 0) + '\0';
    out += std::to_string(misc.size()) + '\0';
    out += misc;
    out += payload;

    return out;
}

bool SaveFileSystem() {
    std::ofstream out(SData::ExternFS::ROM::fileSystem, std::ios::binary);
    if (!out.is_open()) return false;

    for (const std::unique_ptr<Node>& child : FS::root->children) {
        out << SerializeNode(child.get());
    }
    return true;
}

void LoadDir(Node* parent, std::istream& in, const size_t limit) {
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

static bool ReadNullTerminatedField(std::istream& in, std::string& field) {
    field.clear();

    if (std::getline(in, field, '\0'))
        return true;

    if (in.eof() && field.empty())
        return false;

    throw std::runtime_error("Loading filesystem failed; corrupt header");
}

bool LoadNode(Node* parent, std::istream& in) {
    std::string name;
    if (!ReadNullTerminatedField(in, name))
        return false;

    std::string type;
    std::string payloadSizeText;
    std::string sudoFlagText;
    std::string miscSizeText;

    if (!ReadNullTerminatedField(in, type) ||
        !ReadNullTerminatedField(in, payloadSizeText) ||
        !ReadNullTerminatedField(in, sudoFlagText) ||
        !ReadNullTerminatedField(in, miscSizeText))
        throw std::runtime_error("Loading filesystem failed; corrupt header");

    if (name.empty() || type.empty())
        throw std::runtime_error("Loading filesystem failed; corrupt name/type");

    size_t payloadSize;
    size_t miscSize;
    int sudoFlag;

    try {
        payloadSize = std::stoull(payloadSizeText);
        miscSize = std::stoull(miscSizeText);
        sudoFlag = std::stoi(sudoFlagText);
    } catch (const std::exception&) {
        throw std::runtime_error("Loading filesystem failed; corrupt header");
    }

    Metadata md;
    if (sudoFlag != 0 && sudoFlag != 1)
        throw std::runtime_error("Loading filesystem failed; corrupt sudo flag");

    md.sudo = (sudoFlag == 1);

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

bool LoadFileSystem() {
    std::ifstream fs(SData::ExternFS::ROM::fileSystem, std::ios::binary);
    if (!fs.is_open()) return false;

    while (LoadNode(FS::root, fs)) {}
    return true;
}
