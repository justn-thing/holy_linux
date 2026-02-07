#pragma once

inline std::vector<std::string> split(const std::string& s, const char delimiter, const bool preserveEmpty = false,
                                      const bool preserveLast = false) {
    std::vector<std::string> tokens;
    std::string temp;

    for (const char c : s) {
        if (c == delimiter) {
            if (!temp.empty() || preserveEmpty) {
                tokens.push_back(temp);
                temp.clear();
            }
        }
        else {
            temp.push_back(c);
        }
    }

    if (!temp.empty() || preserveLast) {
        tokens.push_back(temp);
    }

    return tokens;
}