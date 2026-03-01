#pragma once

inline int Run() {
    while (true) {
        std::cout << stx::cyan << SData::username << stx::yellow << "@holy-linux "
                  << stx::white << getCosmeticPath() << stx::gray << " $"
                  << stx::reset;

        std::string input;
        std::getline(std::cin, input);
        if (CommandParams params = std::move(parseCommandLine(input));
            Execute(params) == 99)
            return 0;
    }
}
