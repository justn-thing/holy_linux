#include "HolyVim.hpp"

#include <iostream>
#include <sstream>

#include "../ui/Messages.hpp"
#include "../ui/Pages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

void displayIndex(const size_t& index) {
    std::cout << stx::gray << index;
    if (index < 10) {
        std::cout << "  ";
    } else if (index < 100) {
        std::cout << " ";
    }
    std::cout << "> " << stx::reset;
}

size_t displayContent(const std::vector<std::string>& output) {
    const size_t vimIndex = output.size() + 1;

    std::stringstream buffer;
    for (auto prevIndex{1uz}; prevIndex < vimIndex; ++prevIndex) {
        buffer << stx::gray << prevIndex;
        if (prevIndex < 10) {
            buffer << "  ";
        } else if (prevIndex < 100) {
            buffer << " ";
        }
        buffer << "> " << stx::reset << output[prevIndex - 1] << "\n";
    }
    std::cout << buffer.str();
    return vimIndex;
}

void HolyVim(Node* target, const std::string& cmd) {
    std::cout << page::holyVim;

    std::vector<std::string> output;
    auto vimIndex = 1uz;

    if (cmd == "edit") {
        const std::vector<std::string> prevContent = split(target->value, '\n', true);
        vimIndex = displayContent(prevContent);
        output = prevContent;
    }

    std::string temp;
    while (true) {
        displayIndex(vimIndex);

        std::getline(std::cin, temp);
        const std::vector<std::string> cmds = split(temp, ' ', true, true);

        if (cmds[0] == ":goto" || cmds[0] == ":gt" ||
            cmds[0] == ":insert" || cmds[0] == ":ins" ||
            cmds[0] == ":delete" || cmds[0] == ":del") {
            if (cmds.size() < 2) {
                alert(msg::arg_missing, stx::yellow);
                continue;
            }
            std::stringstream stringToSize_t(cmds[1]);

            if (size_t convertedStr; stringToSize_t >> convertedStr) {
                if (convertedStr > 0 && convertedStr <= output.size()) {
                    if (cmds[0] == ":goto" || cmds[0] == ":gt") {
                        vimIndex = convertedStr;
                    }
                    if (cmds[0] == ":insert" || cmds[0] == ":ins") {
                        output.insert(output.begin() + static_cast<int>(convertedStr) - 1, "");
                        vimIndex = convertedStr;
                    }
                    if (cmds[0] == ":delete" || cmds[0] == ":del") {
                        output.erase(output.begin() + static_cast<int>(convertedStr) - 1);
                        --vimIndex;
                    }
                } else
                    alert(msg::invalid_arg, stx::yellow);
            } else
                alert(msg::invalid_arg, stx::yellow);
            continue;
        }
        if (cmds[0] == ":reload" || cmds[0] == ":re") {
            std::cout << stx::move_to_top;
            displayContent(output);
            continue;
        }
        if (cmds[0] == ":commands" || cmds[0] == ":cmds") {
            std::cout << page::holyVimHelp;
            continue;
        }
        if (cmds[0] == ":save" || cmds[0] == ":s") {
            std::string finalOutput;
            for (const std::string& line : output)
                finalOutput += line + "\n";

            target->value = finalOutput;
            break;
        }
        if (cmds[0] == ":discard" || cmds[0] == ":d")
            break;

        if (vimIndex > output.size())
            output.push_back(temp);
        else
            output[vimIndex - 1] = temp;

        ++vimIndex;
    }
}
