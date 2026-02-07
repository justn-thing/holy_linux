#pragma once
#include <string_view>

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
}

namespace err /* error */ {
    constexpr std::string_view not_sudo = "Insufficient permission.\n";
    constexpr std::string_view dir_not_empty = "Directory is not empty.\n";
    constexpr std::string_view dir_empty = "Directory is empty.\n";
    constexpr std::string_view dir_not_found = "Directory not found.\n";
    constexpr std::string_view dir_alr_exists = "Directory already exists.\n";
    constexpr std::string_view file_not_found = "File not found.\n";
    constexpr std::string_view file_alr_exists = "File already exists.\n";
    constexpr std::string_view exec_not_found = "Executing file not found.\n";
    constexpr std::string_view _not_found = " not found.\n";
    constexpr std::string_view invalid_abs_path = "Invalid absolute path.\n";
    constexpr std::string_view invalid_arg = "Invalid argument.\n";
    constexpr std::string_view invalid_file_type = "Invalid file type.\n";
    constexpr std::string_view unknown_file_type = "Unknown file type.\n";
    constexpr std::string_view arg_missing = "Required argument missing.\n";
    constexpr std::string_view cant_remove_self = "Cannot remove current directory.\n";
    constexpr std::string_view cant_remove_root = "Cannot remove root.\n";
    constexpr std::string_view cant_rename_root = "Cannot rename root.\n";
    constexpr std::string_view cant_move_root = "Cannot move root.\n";
    constexpr std::string_view mkfile_dir = "Cannot create directories with mkfile.\n";
    constexpr std::string_view rmfile_dir = "Cannot remove directories with rmfile.\n";
    constexpr std::string_view fail_load_fetch = "Failed to load fetch text file.\n\n";
    constexpr std::string_view fail_load_help = "Failed to load help page text file.\n\n";
    constexpr std::string_view fail_load_holyvim = "Failed to load Holy Vim help page text file.\n\n";
    constexpr std::string_view fail_load_startupcfg = "Failed to load startup config file.\n\n";
    constexpr std::string_view fail_save_startupcfg = "Failed to save startup config file.\n\n";
    constexpr std::string_view fail_mount = "Failed to mount file.\n";
    constexpr std::string_view fail_compile = "Failed to compile file.\n";
    constexpr std::string_view mnt_doesnt_exist = "/root/mnt does not exist.\n";
    constexpr std::string_view unknown_cmd = "Unknown command.\n";
    constexpr std::string_view fail_confirm = "Verification failed.\n";
}

inline void throwError(const std::string_view message, const std::string_view color, const bool startup = false, const std::string &opt = "") {
    if (startup) {
        return;
    }
    std::cout << color << opt << message << stx::reset;
}