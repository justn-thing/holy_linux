#pragma once

#include <sstream>
#include <string>
#include <utility>

#include "../session/Login.hpp"
#include "../session/SessionData.hpp"
#include "../cmd/CommandParams.hpp"

inline CommandParams parseCommandLine(const std::string &input) {
    CommandParams result;
    result.raw = input;

    std::stringstream ss(input);
    ss >> result.cmd;
    if (result.cmd == "sudo") {
        if (SData::root || LoginRoot())
            result.sudo = true;

        if (!(ss >> result.cmd)) {
            result.cmd.clear();
            return result;
        }
    }

    std::string temp;
    if (!(ss >> temp))
        return result;

    if (temp.starts_with('-'))
        result.flags = temp.substr(1);
    else
        result.args.push_back(std::move(temp));

    while (ss >> temp)
        result.args.push_back(std::move(temp));

    return result;
}
