#pragma once

#include <string>
#include <string_view>

namespace msg {
    extern const std::string_view not_sudo;
    extern const std::string_view dir_not_empty;
    extern const std::string_view dir_empty;
    extern const std::string_view dir_not_found;
    extern const std::string_view dir_alr_exists;
    extern const std::string_view file_not_found;
    extern const std::string_view file_alr_exists;
    extern const std::string_view exec_not_found;
    extern const std::string_view _not_found;
    extern const std::string_view invalid_path;
    extern const std::string_view invalid_arg;
    extern const std::string_view invalid_file_type;
    extern const std::string_view unknown_file_type;
    extern const std::string_view arg_missing;
    extern const std::string_view cant_remove_self;
    extern const std::string_view cant_move_root;
    extern const std::string_view mkfile_dir;
    extern const std::string_view rmfile_dir;
    extern const std::string_view mkdir_file;
    extern const std::string_view rmdir_file;
    extern const std::string_view fail_load_fetch;
    extern const std::string_view fail_load_help;
    extern const std::string_view fail_load_holyvim;
    extern const std::string_view startupcfg_not_exist;
    extern const std::string_view fail_save_startupcfg;
    extern const std::string_view fail_load_filesystem;
    extern const std::string_view fail_save_filesystem;
    extern const std::string_view fail_mount;
    extern const std::string_view fail_compile;
    extern const std::string_view mnt_doesnt_exist;
    extern const std::string_view unknown_cmd;
    extern const std::string_view fail_confirm;
    extern const std::string_view cant_remove_root;
    extern const std::string_view cant_rename_root;
    extern const std::string_view pass_set_fail;
    extern const std::string_view begin_boot;
    extern const std::string_view begin_poweroff;
    extern const std::string_view begin_load_fs;
    extern const std::string_view begin_save_fs;
    extern const std::string_view begin_startupcfg;
    extern const std::string_view begin_save_startupcfg;
}

void alert(std::string_view message, std::string_view color, bool startup = false, const std::string& opt = "");
