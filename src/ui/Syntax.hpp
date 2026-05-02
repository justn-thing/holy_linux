#pragma once

#include <string_view>

namespace stx {
    extern const std::string_view reset;
    extern const std::string_view black;
    extern const std::string_view red;
    extern const std::string_view green;
    extern const std::string_view yellow;
    extern const std::string_view blue;
    extern const std::string_view magenta;
    extern const std::string_view cyan;
    extern const std::string_view white;
    extern const std::string_view gray;
    extern const std::string_view bg_black;
    extern const std::string_view bg_red;
    extern const std::string_view bg_green;
    extern const std::string_view bg_yellow;
    extern const std::string_view bg_blue;
    extern const std::string_view bg_magenta;
    extern const std::string_view bg_cyan;
    extern const std::string_view bg_white;
    extern const std::string_view bg_gray;
    extern const std::string_view move_to_top;
    extern const std::string_view clear;

    void ClearConsole();
}
