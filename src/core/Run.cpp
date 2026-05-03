#include "Run.hpp"

#include <iostream>
#include <string>

#include "../cmd/CommandParser.hpp"
#include "../cmd/Execution.hpp"
#include "../fs/FileTree.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Syntax.hpp"

void PrintShellPrompt() {
    std::cout << stx::cyan << SData::username << stx::yellow << "@holy-linux "
              << stx::white << GetCosmeticPath() << stx::gray << " $"
              << stx::reset;
}

int Run() {
    while (true) {
        PrintShellPrompt();

        std::string input;
        std::getline(std::cin, input);

        if (CommandParams params = ParseCommandLine(input);
            Execute(params) == 99)
            return 0;

        if (!input.empty())
            SData::cmdHistory.emplace_back(input);
    }
}
