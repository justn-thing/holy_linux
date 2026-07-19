#include "FileTree.hpp"

#include "../session/SessionData.hpp"

namespace FS {
    Node* root = new Node("dir", "root");
    Node* current = root;

    void Reset() {
        delete root;
        root = new Node("dir", "root");
        current = root;
    }
}
