#include "Run.hpp"

#include <iostream>
#include <string>

#include "../cmd/CmdParser.hpp"
#include "../cmd/Execution.hpp"
#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Syntax.hpp"
#include "../util/ReturnCodes.hpp"

namespace {
    void PrintShellPrompt() {
        std::string buffer;
        buffer += stx::cyan;
        buffer += SData::user.name;
        buffer += stx::yellow;
        buffer += "@holy-linux ";
        buffer += stx::gray;
        buffer += '[';
        buffer += stx::white;
        buffer += FS::current->GetCosmeticPath();
        buffer += stx::gray;
        buffer += ']';
        buffer += stx::gray;
        buffer += " $";
        buffer += stx::reset;

        std::cout << buffer;
    }
}

int Run() {
    while (true) {
        PrintShellPrompt();

        std::string input;
        std::getline(std::cin, input);

        CmdParams params = ParseCommandLine(input);
        if (const int returnCode = ExecuteCmdLine(params);
            returnCode == Poweroff || returnCode == Reboot) {
            return returnCode;
        }

        if (!input.empty()) {
            SData::user.cmdHistory.emplace_back(input);
        }
    }
}
