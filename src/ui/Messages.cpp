#include "Messages.hpp"

#include <iostream>

#include "../ui/Syntax.hpp"

namespace msg {
    const std::string_view not_sudo = "Insufficient permission.\n";
    const std::string_view dir_not_empty = "Directory is not empty.\n";
    const std::string_view dir_empty = "Directory is empty.\n";
    const std::string_view dir_not_found = "Directory not found.\n";
    const std::string_view dir_alr_exists = "Directory already exists.\n";
    const std::string_view file_not_found = "File not found.\n";
    const std::string_view file_alr_exists = "File already exists.\n";
    const std::string_view exec_not_found = "Executing file not found.\n";
    const std::string_view _not_found = " not found.\n";
    const std::string_view invalid_path = "Invalid path.\n";
    const std::string_view invalid_arg = "Invalid argument.\n";
    const std::string_view invalid_file_type = "Invalid file type.\n";
    const std::string_view unknown_file_type = "Unknown file type.\n";
    const std::string_view arg_missing = "Required argument missing.\n";
    const std::string_view cant_remove_self = "Cannot remove current directory.\n";
    const std::string_view cant_move_root = "Cannot move root.\n";
    const std::string_view mkfile_dir = "Cannot create directories with mkfile.\n";
    const std::string_view rmfile_dir = "Cannot remove directories with rmfile.\n";
    const std::string_view mkdir_file = "Cannot create files with mkdir.\n";
    const std::string_view rmdir_file = "Cannot remove files with rmdir.\n";
    const std::string_view fail_load_fetch = "Failed to load fetch text file.\n\n";
    const std::string_view fail_load_help = "Failed to load help page text file.\n\n";
    const std::string_view fail_load_holyvim = "Failed to load Holy Vim help page text file.\n\n";
    const std::string_view startupcfg_not_exist = "Startup config file does not exist, creating it...\n\n";
    const std::string_view fail_save_startupcfg = "Failed to save startup config file.\n\n";
    const std::string_view fail_load_filesystem = "Failed to load filesystem file.\n\n";
    const std::string_view fail_save_filesystem = "Failed to save filesystem file.\n\n";
    const std::string_view fail_mount = "Failed to mount file.\n";
    const std::string_view fail_compile = "Failed to compile file.\n";
    const std::string_view mnt_doesnt_exist = "/root/mnt directory does not exist, creating it...\n";
    const std::string_view unknown_cmd = "Unknown command.\n";
    const std::string_view fail_confirm = "Verification failed.\n";
    const std::string_view cant_remove_root = "Cannot remove root.\n";
    const std::string_view cant_rename_root = "Cannot rename root.\n";
    const std::string_view pass_set_fail = "Passwords don't match. Try again.\n";
    const std::string_view begin_boot = "Booting...\n";
    const std::string_view begin_poweroff = "System shutdown in progress.\n";
    const std::string_view begin_load_fs = "Loading filesystem...\n";
    const std::string_view begin_save_fs = "Saving filesystem...\n";
    const std::string_view begin_startupcfg = "Beginning startup config...\n";
    const std::string_view begin_save_startupcfg = "Saving startup config...\n";
}

void alert(const std::string_view message, const std::string_view color, const bool startup, const std::string& opt) {
    if (startup) return;

    std::cout << color << opt << message << stx::reset;
}
