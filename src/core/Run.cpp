#include "Run.hpp"

#include <iostream>
#include <string>

#include "../cmd/CommandParser.hpp"
#include "../cmd/Execution.hpp"
#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Syntax.hpp"

void PrintShellPrompt() {
    std::string buffer;
    buffer += stx::cyan;
    buffer += SData::username;
    buffer += stx::yellow;
    buffer += "@holy-linux ";
    buffer += stx::gray;
    buffer += '<';
    buffer += stx::white;
    buffer += GetCosmeticPath();
    buffer += stx::gray;
    buffer += '>';
    buffer += stx::gray;
    buffer += " $";
    buffer += stx::reset;

    std::cout << buffer;
}

int Run() {
    while (true) {
        PrintShellPrompt();

        std::string input;
        std::getline(std::cin, input);

        CommandParams params = ParseCommandLine(input);
        if (const int& returnCode = Execute(params); returnCode != 0)
            return returnCode;

        if (!input.empty())
            SData::cmdHistory.emplace_back(input);
    }
}
