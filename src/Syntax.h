#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

namespace stx /* syntax */ {
    constexpr std::string_view reset = "\033[0m";
    constexpr std::string_view black = "\033[30m";
    constexpr std::string_view red = "\033[31m";
    constexpr std::string_view green = "\033[32m";
    constexpr std::string_view yellow = "\033[33m";
    constexpr std::string_view blue = "\033[34m";
    constexpr std::string_view magenta = "\033[35m";
    constexpr std::string_view cyan = "\033[36m";
    constexpr std::string_view white = "\033[37m";
    constexpr std::string_view gray = "\033[90m";
    constexpr std::string_view bg_black = "\033[40m";
    constexpr std::string_view bg_red = "\033[41m";
    constexpr std::string_view bg_green = "\033[42m";
    constexpr std::string_view bg_yellow = "\033[43m";
    constexpr std::string_view bg_blue = "\033[44m";
    constexpr std::string_view bg_magenta = "\033[45m";
    constexpr std::string_view bg_cyan = "\033[46m";
    constexpr std::string_view bg_white = "\033[47m";
    constexpr std::string_view bg_gray = "\033[100m";
    constexpr std::string_view clear = "\033[2J\033[H";

    inline void clearConsole() {
        #ifdef _WIN32
            const auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
            if (handle == INVALID_HANDLE_VALUE) return;

            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (!GetConsoleScreenBufferInfo(handle, &csbi)) return;

            const DWORD size = static_cast<DWORD>(csbi.dwSize.X) * static_cast<DWORD>(csbi.dwSize.Y);
            DWORD written = 0;
            constexpr COORD home = {0, 0};

            FillConsoleOutputCharacter(handle, ' ', size, home, &written);
            FillConsoleOutputAttribute(handle, csbi.wAttributes, size, home, &written);
            SetConsoleCursorPosition(handle, home);
        #else
                std::cout << clear;
        #endif
    }
}
