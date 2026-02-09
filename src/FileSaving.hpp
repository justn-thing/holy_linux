#pragma once

inline void loadBootCfg(Node* root) {
    Node* boot = getChild(root, "boot", "dir");
    if (!boot) {
        throwError(err::fail_load_startupcfg, stx::red);
        return;
    }

    Node* config = getChild(boot, "startupConfig", "cmd");
    if (!config) {
        throwError(err::fail_load_startupcfg, stx::red);
        return;
    }

    ifstream filein("rom/startup.txt");
    if (!filein.is_open()) {
        throwError(err::fail_load_startupcfg, stx::red);
        return;
    }

    stringstream ss;
    ss << filein.rdbuf();
    config->value = ss.str();
    filein.close();
}

inline string serializeNode(const Node* node);

inline string serializeDir(const Node* dir) {
    string value;
    for (const unique_ptr<Node>& child : dir->children) {
        value += serializeNode(child.get());
    }
    return value;
}

inline string serializeNode(const Node* node) {
    string payload;

    if (node->type == "dir") {
        payload = serializeDir(node);
    } else {
        payload = node->value;
    }

    const string& misc = node->metadata.misc;

    string out;
    out += node->name + "." + node->type + " ";
    out += to_string(payload.size()) + " ";
    out += to_string(node->metadata.sudo ? 1 : 0) + " ";
    out += to_string(misc.size()) + "\n";
    out += misc;
    out += payload;

    return out;
}

inline bool saveFilesystem(const Node* root) {
    ofstream out("rom/fileSystem.txt", ios::binary);
    if (!out.is_open()) return false;

    for (const unique_ptr<Node>& child : root->children) {
        out << serializeNode(child.get());
    }
    return true;
}

inline bool loadNode(Node* parent, istream& in);

inline void loadDir(Node* parent, istream& in, size_t limit) {
    string buf(limit, '\0');

    /*if (limit > static_cast<size_t>(numeric_limits<streamsize>::max())) {
        throw runtime_error("dir payload too large");
    }*/

    in.read(buf.data(), static_cast<streamsize>(limit));
    /*if (static_cast<size_t>(in.gcount()) != limit) {
        throw runtime_error("unexpected EOF in directory");
    }*/

    istringstream sub(buf);
    while (loadNode(parent, sub)) {}
}

inline bool loadNode(Node* parent, istream& in)
{
    string header;
    if (!getline(in, header))
        return false; // clean EOF

    if (header.empty())
        return false; // ignore trailing newline safely

    string nameType;
    size_t payloadSize;
    int sudoFlag;
    size_t miscSize;

    {
        stringstream ss(header);
        /*if (!(ss >> nameType >> payloadSize >> sudoFlag >> miscSize))
            throw runtime_error("corrupt header");*/
    }

    const size_t dot = nameType.rfind('.');
    /*if (dot == string::npos || dot == 0 || dot == nameType.size() - 1)
        throw runtime_error("corrupt name/type");*/

    const string name = nameType.substr(0, dot);
    const string type = nameType.substr(dot + 1);

    Metadata md;
    md.sudo = (sudoFlag != 0);

    // read misc metadata
    md.misc.resize(miscSize);
    in.read(md.misc.data(), static_cast<streamsize>(miscSize));
    /*if (static_cast<size_t>(in.gcount()) != miscSize)
        throw runtime_error("unexpected EOF in metadata");*/

    Node* node = newChild(parent, name, type, md.sudo, md.misc);

    if (type == "dir") {
        loadDir(node, in, payloadSize);
    } else {
        node->value.resize(payloadSize);
        in.read(node->value.data(), static_cast<streamsize>(payloadSize));

        /*if (static_cast<size_t>(in.gcount()) != payloadSize)
            throw runtime_error("unexpected EOF in file");*/
    }

    return true;
}

inline bool loadFilesystem(Node* root) {
    ifstream fs("rom/fileSystem.txt", ios::binary);
    if (!fs.is_open()) return false;

    while (loadNode(root, fs)) {}
    return true;
}
