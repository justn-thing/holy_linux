#include "CommandParser.hpp"

#include <sstream>
#include <utility>

#include "../session/Login.hpp"
#include "../session/SessionData.hpp"

CommandParams ParseCommandLine(const std::string& input) {
    CommandParams result;
    result.raw = input;

    std::istringstream ss(input);
    ss >> result.cmd;
    if (result.cmd == "sudo") {
        if (SData::root || LoginRoot()) {
            result.sudo = true;
        }

        if (!(ss >> result.cmd)) {
            result.cmd.clear();
            return result;
        }
    }

    std::string temp;
    std::string tempLong;
    bool literalMode = false;
    bool openQuotes = false;

    while (ss >> temp) {
        if (openQuotes) {
            if (temp.back() == '"') {
                tempLong += temp.substr(0, temp.size() - 1);
                result.args.emplace_back(std::move(tempLong));
                tempLong.clear();
                openQuotes = false;
            } else {
                tempLong += temp + ' ';
            }
        } else if (temp[0] == '"') {
            if (temp.back() == '"' && temp.size() > 1) {
                result.args.emplace_back(temp.substr(1, temp.size() - 2));
            } else {
                tempLong += temp.substr(1) + ' ';
                openQuotes = true;
            }
        } else if (!literalMode && temp == "--") {
            literalMode = true;
        } else if (!literalMode && temp[0] == '-') {
            result.flags.emplace_back(std::move(temp));
        } else {
            result.args.emplace_back(std::move(temp));
        }
    }

    if (!tempLong.empty())
        result.args.emplace_back(std::move(tempLong));

    return result;
}
