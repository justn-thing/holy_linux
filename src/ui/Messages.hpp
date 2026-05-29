#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <string_view>

#include "../session/SessionData.hpp"

namespace msg {
    class OutputStream {
    public:
        template<typename T>
        OutputStream& operator<<(const T& value) {
            if (SData::redirectTarget) {
                std::ostringstream ss;
                ss << value;
                SData::redirectTarget->value += ss.str();
            } else {
                std::cout << value;
            }

            return *this;
        }

        OutputStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
            if (SData::redirectTarget) {
                std::ostringstream ss;
                manip(ss);
                SData::redirectTarget->value += ss.str();
            } else {
                manip(std::cout);
            }

            return *this;
        }
    };

    extern OutputStream cout;

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
    extern const std::string_view copy_into_descendant;
    extern const std::string_view move_into_descendant;
    extern const std::string_view copy_into_file;
    extern const std::string_view move_into_file;
    extern const std::string_view invalid_username;
    extern const std::string_view unknown_flag;
    extern const std::string_view cmd_recursion;
}

void alert(const std::string_view& message, const std::string_view& color);
