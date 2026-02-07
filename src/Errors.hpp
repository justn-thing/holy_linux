#pragma once

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
    constexpr std::string_view cant_move_root = "Cannot move root.\n";
    constexpr std::string_view mkfile_dir = "Cannot create directories with mkfile.\n";
    constexpr std::string_view rmfile_dir = "Cannot remove directories with rmfile.\n";
    constexpr std::string_view fail_load_fetch = "Failed to load fetch text file.\n\n";
    constexpr std::string_view fail_load_help = "Failed to load help page text file.\n\n";
    constexpr std::string_view fail_load_holyvim = "Failed to load Holy Vim help page text file.\n\n";
    constexpr std::string_view fail_load_startupcfg = "Failed to load startup config file.\n\n";
    constexpr std::string_view fail_save_startupcfg = "Failed to save startup config file.\n\n";
    constexpr std::string_view fail_load_filesystem = "Failed to load filesystem file.\n\n";
    constexpr std::string_view fail_save_filesystem = "Failed to save filesystem file.\n\n";
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
