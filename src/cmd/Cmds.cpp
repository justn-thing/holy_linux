#include "Cmds.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "../apps/HolyVim.hpp"
#include "../cmd/CmdParams.hpp"
#include "../cmd/CmdParser.hpp"
#include "../cmd/CmdUtils.hpp"
#include "../cmd/Execution.hpp"
#include "../fs/FileSaving.hpp"
#include "../fs/FileTree.hpp"
#include "../session/Login.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Pages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"
#include "../util/ReturnCodes.hpp"

namespace {
    namespace cmds {
        int cd_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            if (Node* target = GetAbsolute(param.args[0]);
                target && target->_type == "dir") {
                if (target->_metadata.sudo && NotSudo(param)) return 1;

                FS::current = target;
                } else {
                    alert(msg::dir_not_found, stx::red);
                    return 1;
                }

            return 0;
        }

        int dir_ls_(const CmdParams& param) {
            if (param.args.empty()) {
                FS::current->DisplayDir();
            } else if (const Node* target = GetAbsolute(param.args[0])) {
                target->DisplayDir();
            } else {
                alert(msg::dir_not_found, stx::red);
                return 1;
            }

            return 0;
        }

        int mkdir_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            const size_t index = param.args[0].rfind('/');
            std::string path;
            std::string name;

            if (index == std::string::npos) {
                name = param.args[0];
            } else if (index == param.args[0].size() - 1) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            } else {
                path = param.args[0].substr(0, index + 1);
                name = param.args[0].substr(index + 1);
            }

            if (name.contains('.') || name.empty()) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            }

            if (Node* target = GetAbsolute(path);
                target && target->_type == "dir") {
                if (!target->NewChild(name, "dir")) {
                    alert(msg::dir_alr_exists, stx::yellow);
                    return 1;
                }
                } else {
                    alert(msg::invalid_path, stx::red);
                    return 1;
                }

            return 0;
        }

        int rmdir_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            bool recursive = false;
            for (const char& flag : param.shortFlags) {
                if (flag == 'r') recursive = true;
                else {
                    alert(msg::unknown_flag, stx::yellow);
                    return 1;
                }
            }

            const Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::dir_not_found, stx::red);
                return 1;
            } if (target == FS::root) {
                alert(msg::cant_remove_root, stx::yellow);
                return 1;
            } if (target == FS::current || (recursive && target->IsAncestorOf(FS::current))) {
                alert(msg::cant_remove_self, stx::yellow);
                return 1;
            }
            if (target->ContainsLockedNode() && NotSudo(param)) return 1;

            if (target->_type != "dir") {
                alert(msg::rmdir_file, stx::yellow);
                return 1;
            }

            if (!target->Remove(recursive)) {
                return 1;
            }

            return 0;
        }

        int mkfile_touch_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            const size_t index = param.args[0].rfind('/');
            std::string path;
            std::string name;
            std::string type;

            if (index == std::string::npos) {
                name = param.args[0];
            } else {
                path = param.args[0].substr(0, index + 1);
                name = param.args[0].substr(index + 1);
            }

            if (const size_t dotIndex = name.rfind('.');
                dotIndex == std::string::npos) {
                type = "txt";
                } else {
                    type = name.substr(dotIndex + 1);
                    name = name.substr(0, dotIndex);
                }
            if (name.contains('.') || name.empty() || type.empty()) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            }

            if (Node* target = GetAbsolute(path);
                target && target->_type == "dir") {
                if (!target->NewChild(name, type)) {
                    alert(msg::file_alr_exists, stx::yellow);
                    return 1;
                }
                } else {
                    alert(msg::invalid_path, stx::red);
                    return 1;
                }

            return 0;
        }

        int rmfile_rm_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            const Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            } if (target->_type == "dir") {
                alert(msg::rmfile_dir, stx::yellow);
                return 1;
            }
            if (target->_metadata.sudo && NotSudo(param)) return 1;

            if (!target->Remove()) {
                return 1;
            }

            return 0;
        }

        int rename_(const CmdParams& param) {
            if (NotEnoughArgs(param, 2)) return 1;

            if (param.args[1].contains('/')) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            }

            Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            } if (target == FS::root) {
                alert(msg::cant_rename_root, stx::yellow);
                return 1;
            }
            if (target->_metadata.sudo && NotSudo(param)) return 1;

            const size_t index = param.args[1].rfind('.');
            std::string name;
            std::string type;

            if (target->_type == "dir" && index != std::string::npos) {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }

            if (index == std::string::npos) {
                name = param.args[1];
                type = target->_type;
            } else {
                name = param.args[1].substr(0, index);
                type = param.args[1].substr(index + 1);
                if (type == "dir") {
                    alert(msg::invalid_file_type, stx::yellow);
                    return 1;
                }
            }

            if (name.contains('.') || name.empty() || type.empty()) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            }

            if (target->_parent->GetChild(name, type)) {
                alert(msg::file_alr_exists, stx::yellow);
                return 1;
            }

            target->_name = name;
            target->_type = type;

            return 0;
        }

        int write_wr_edit_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            }

            if (target->_metadata.sudo && NotSudo(param)) return 1;

            if (!IsTextFileType(target->_type)) {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }

            HolyVim(target, param.cmd);

            return 0;
        }

        int read_cat_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            const Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            }

            if (target->_metadata.sudo && NotSudo(param)) return 1;

            if (!IsTextFileType(target->_type)) {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }

            msg::cout << target->_value << '\n';

            return 0;
        }

        int execute_exec_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            const Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            }
            if (target->_metadata.sudo && NotSudo(param)) return 1;
            if (!IsExecutableFileType(target->_type)) {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }
            if (IsOverflowingStack(target)) return 1;

            AddToStack(target);

            if (target->_type == "cmd") {
                for (const std::string& line : split(target->_value, '\n')) {
                    CmdParams params = ParseCommandLine(line);
                    if (const int& returnCode = ExecuteCmdLine(params); returnCode != 0) {
                        RemoveFromStack(target);
                        return returnCode;
                    }
                }
            } else if (target->_type == "py") {
                std::ofstream fileout(SData::ExternFS::RAM::py);
                fileout << target->_value;
                fileout.close();

#ifdef _WIN32
                const std::wstring runCmd = L"py \"" + SData::ExternFS::RAM::py.wstring() + L"\"";
                const int runCode = _wsystem(runCmd.c_str());
#else
                const std::string runCmd = "python3 \"" + SData::RAM::py.string() + "\"";
                const int runCode = system(runCmd.c_str());
#endif

                std::ofstream fileErase(SData::ExternFS::RAM::py);
                fileErase.close();
                if (runCode != 0) {
                    RemoveFromStack(target);
                    return 1;
                }
            } else if (target->_type == "exe") {
                std::ofstream fileout(SData::ExternFS::RAM::exe, std::ios::binary);
                fileout << target->_value;
                fileout.close();
#ifdef _WIN32
                const std::wstring runCmd = L"\"" + SData::ExternFS::RAM::exe.wstring() + L"\"";
                const int runCode = _wsystem(runCmd.c_str());
#else
                const std::string runCmd = "chmod +x \"" + SData::RAM::exe.string() + "\" && \"" +
                                           SData::RAM::exe.string() + "\"";
                const int runCode = system(runCmd.c_str());
#endif
                if (runCode != 0) {
                    RemoveFromStack(target);
                    return 1;
                }
            }

            RemoveFromStack(target);
            return 0;
        }

        int compile_comp_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            const Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            }
            if (target->_metadata.sudo && NotSudo(param)) return 1;

            if (target->_type == "cpp") {
                std::ofstream fileout(SData::ExternFS::RAM::cpp);
                fileout << target->_value;
                fileout.close();

#ifdef _WIN32
                const std::filesystem::path compiledPath = SData::ExternFS::RAM::cpp.parent_path() / "cppCompiled.exe";
                const std::wstring compileCmd = L"g++ \"" + SData::ExternFS::RAM::cpp.wstring() + L"\" -o \"" +
                                                compiledPath.wstring() + L"\"";
#else
                const std::filesystem::path compiledPath = SData::RAM::cpp.parent_path() / "cppCompiled";
                const std::string compileCmd = "g++ \"" + SData::RAM::cpp.string() + "\" -o \"" +
                                               compiledPath.string() + "\"";
#endif

                if (
    #ifdef _WIN32
                    _wsystem(compileCmd.c_str())
    #else
                    system(compileCmd.c_str())
    #endif
                    != 0) {
                    alert(msg::fail_compile, stx::red);
                    return 1;
                    }

                std::ofstream fileErase(SData::ExternFS::RAM::cpp);
                fileErase.close();

                std::ifstream filein(compiledPath, std::ios::binary);
                if (!filein.is_open()) {
                    alert(msg::fail_compile, stx::red);
                    return 1;
                }
                auto output = std::string(std::istreambuf_iterator(filein), std::istreambuf_iterator<char>());
                filein.close();
                std::filesystem::remove(compiledPath);

                target->_parent->RemoveChild(target->_name, "exe");
                Node* exeFile = target->_parent->NewChild(target->_name, "exe");
                exeFile->_value = std::move(output);
            } else {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }

            return 0;
        }

        int mount_mnt_(const CmdParams& param) {
            if (NoArgs(param)) return 1;
            if (NotSudo(param)) return 1;

            const std::string fileName = std::filesystem::path(param.args[0]).filename().string();
            const size_t index = fileName.rfind('.');
            if (index == std::string::npos) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            }

            const std::string name = fileName.substr(0, index);
            const std::string type = fileName.substr(index + 1);
            if (name.contains('.') || name.empty() || type.empty()) {
                alert(msg::invalid_arg, stx::yellow);
                return 1;
            }

            if (!IsTextFileType(type)) {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }

            std::ifstream filein(param.args[0], std::ios::binary);
            if (!filein.is_open()) {
                alert(msg::fail_mount, stx::red);
                return 1;
            }

            Node* mount = FS::root->GetChild("mnt", "dir");
            if (!mount) {
                mount = FS::root->NewChild("mnt", "dir", true);
            }

            if (Node* mountedFile = mount->NewChild(name, type)) {
                filein.seekg(0, std::ios::end);
                std::string data;
                data.resize(filein.tellg());

                filein.seekg(0, std::ios::beg);
                filein.read(data.data(), static_cast<std::streamsize>(data.size()));

                mountedFile->_value = std::move(data);
            } else {
                alert(msg::file_alr_exists, stx::yellow);
                return 1;
            }

            return 0;
        }

        int export_(const CmdParams& param) {
            if (NoArgs(param)) return 1;
            if (NotSudo(param)) return 1;

            const Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }
            if (target->_type == "dir") {
                alert(msg::invalid_file_type, stx::yellow);
                return 1;
            }

            const std::filesystem::path newFile = SData::ExternFS::Export::selfDir / (target->_name + '.' + target->_type);

            std::ofstream fileout(newFile, std::ios::binary);
            if (!fileout.is_open()) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }

            fileout.write(target->_value.data(), static_cast<std::streamsize>(target->_value.size()));

            return 0;
        }

        int copy_cp_(const CmdParams& param) {
            if (NotEnoughArgs(param, 2)) return 1;

            const Node* fromNode = GetAbsolute(param.args[0]);
            if (!fromNode) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }

            Node* toNode = GetAbsolute(param.args[1]);
            if (!toNode) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }
            if (toNode->_type != "dir") {
                alert(msg::copy_into_file, stx::yellow);
                return 1;
            }

            if (fromNode->IsAncestorOf(toNode)) {
                alert(msg::copy_into_descendant, stx::yellow);
                return 1;
            }

            if (!fromNode->Copy(toNode, param.sudo)) {
                return 1;
            }

            return 0;
        }

        int move_mv_(const CmdParams& param) {
            if (NotEnoughArgs(param, 2)) return 1;

            const Node* fromNode = GetAbsolute(param.args[0]);
            if (!fromNode) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }
            if (fromNode == FS::root) {
                alert(msg::cant_move_root, stx::yellow);
                return 1;
            }

            Node* toNode = GetAbsolute(param.args[1]);
            if (!toNode) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }
            if (toNode->_type != "dir") {
                alert(msg::move_into_file, stx::yellow);
                return 1;
            }

            if (fromNode->IsAncestorOf(toNode)) {
                alert(msg::move_into_descendant, stx::yellow);
                return 1;
            }

            if (!fromNode->Move(toNode, param.sudo)) {
                return 1;
            }

            return 0;
        }

        int lock_unlock_(const CmdParams& param) {
            if (NoArgs(param)) return 1;
            if (NotSudo(param)) return 1;

            Node* target = GetAbsolute(param.args[0]);
            if (!target) {
                alert(msg::file_not_found, stx::red);
                return 1;
            }

            bool recursive = false;
            for (const char& flag : param.shortFlags) {
                if (flag == 'r') recursive = true;
                else {
                    alert(msg::unknown_flag, stx::yellow);
                    return 1;
                }
            }

            target->SetLock(param.cmd == "lock", recursive);

            return 0;
        }

        int password_passwd_(const CmdParams& param) {
            if (NotEnoughArgs(param, 2)) return 1;

            bool root = false;
            for (const std::string& flag : param.longFlags) {
                if (flag == "root") {
                    if (param.sudo) root = true;
                    else {
                        alert(msg::not_sudo, stx::yellow);
                        return 1;
                    }
                }
                else {
                    alert(msg::unknown_flag, stx::yellow);
                    return 1;
                }
            }

            if (param.args[0].empty() || param.args[0] != param.args[1]) {
                alert(msg::pass_set_fail, stx::yellow);
                return 1;
            }

            ChangePassword(root ? "//root//" : SData::user.name, param.args[0]);

            return 0;
        }

        int history_(const CmdParams& param) {
            for (const std::string& cmd : SData::user.cmdHistory) {
                msg::cout << cmd << '\n';
            }

            return 0;
        }

        int echo_(const CmdParams& param) {
            for (const std::string& word : param.args) {
                msg::cout << word << " ";
            }
            msg::cout << '\n';

            return 0;
        }

        int help_(const CmdParams& param) {
            msg::cout << page::help << '\n';

            return 0;
        }

        int clear_cls_(const CmdParams& param) {
            stx::ClearConsole();

            return 0;
        }

        int fetch_(const CmdParams& param) {
            msg::cout << page::fetch << '\n';

            return 0;
        }

        int pwd_(const CmdParams& param) {
            msg::cout << FS::current->GetPath() << '\n';

            return 0;
        }

        int whoami_(const CmdParams& param) {
            msg::cout << SData::user.name << '\n';

            return 0;
        }

        int date_(const CmdParams& param) {
            const auto now = std::chrono::system_clock::now();
            const std::time_t date = std::chrono::system_clock::to_time_t(now);

            msg::cout << std::put_time(std::localtime(&date), "%d/%m/%Y") << '\n';

            return 0;
        }

        int time_(const CmdParams& param) {
            const auto now = std::chrono::system_clock::now();
            const std::time_t time = std::chrono::system_clock::to_time_t(now);

            msg::cout << std::put_time(std::localtime(&time), "%H:%M:%S") << '\n';

            return 0;
        }

        int du_(const CmdParams& param) {
            const Node* node;
            if (param.args.empty()) {
                node = FS::current;
            } else {
                node = GetAbsolute(param.args[0]);

                if (!node) {
                    alert(msg::invalid_path, stx::red);
                    return 1;
                }
            }

            size_t bytes = node->GetFileSize();
            double kB = static_cast<double>(bytes) / 1024.0;
            double MB = kB / 1024.0;
            double GB = MB / 1024.0;
            msg::cout << std::format("{:.2f} GiB / {:.2f} MiB / {:.2f} KiB / {} B", GB, MB, kB, bytes) << '\n';

            return 0;
        }

        int tree_ (const CmdParams& param) {
            if (param.args.empty()) {
                FS::current->PrintTree();
                return 0;
            }

            if (const Node* target = GetAbsolute(param.args[0])) {
                target->PrintTree();
            } else {
                alert(msg::invalid_path, stx::red);
                return 1;
            }

            return 0;
        }

        int find_(const CmdParams& param) {
            if (NoArgs(param)) return 1;

            Node* ancestor;
            if (param.args.size() == 1) {
                ancestor = FS::current;
            } else {
                ancestor = GetAbsolute(param.args[1]);
                if (!ancestor) {
                    alert(msg::invalid_path, stx::yellow);
                    return 1;
                }
            }

            std::vector<Node*> result;
            if (const size_t dot = param.args[0].rfind('.');
                dot == std::string::npos || dot == param.args[0].size() - 1) {
                ancestor->FindDescendants(param.args[0].substr(0, dot), "", result, true);
                } else {
                    ancestor->FindDescendants(param.args[0].substr(0, dot), param.args[0].substr(dot + 1), result);
                }

            if (result.empty()) {
                alert(msg::file_not_found, stx::yellow);
                return 1;
            }

            for (const Node* node : result) {
                msg::cout << node->GetPath() << '\n';
            }

            return 0;
        }

        int poweroff_reboot_(const CmdParams& param) {
            stx::ClearConsole();
            alert(msg::begin_poweroff, stx::green);

            const int returnCode = param.cmd == "poweroff" ? Poweroff : Reboot;

            bool discardChanges = false;
            for (const char& flag : param.shortFlags) {
                if (flag == 'd') discardChanges = true;
                else {
                    alert(msg::unknown_flag, stx::yellow);
                    return 1;
                }
            }

            if (discardChanges) return returnCode;

            alert(msg::begin_save_fs, stx::green);
            if (const bool saveSuccess = SaveFileSystem();
                !saveSuccess) {
                alert(msg::fail_save_filesystem, stx::red);
                }

            return returnCode;
        }
    }
}

const std::unordered_map<std::string_view, int (*)(const CmdParams&)> Commands = {
    {"cd", cmds::cd_},

    {"dir", cmds::dir_ls_},
    {"ls", cmds::dir_ls_},

    {"mkdir", cmds::mkdir_},
    {"rmdir", cmds::rmdir_},

    {"mkfile", cmds::mkfile_touch_},
    {"touch", cmds::mkfile_touch_},

    {"rmfile", cmds::rmfile_rm_},
    {"rm", cmds::rmfile_rm_},

    {"rename", cmds::rename_},

    {"write", cmds::write_wr_edit_},
    {"wr", cmds::write_wr_edit_},
    {"edit", cmds::write_wr_edit_},

    {"read", cmds::read_cat_},
    {"cat", cmds::read_cat_},

    {"execute", cmds::execute_exec_},
    {"exec", cmds::execute_exec_},

    {"compile", cmds::compile_comp_},
    {"comp", cmds::compile_comp_},

    {"mount", cmds::mount_mnt_},
    {"mnt", cmds::mount_mnt_},

    {"export", cmds::export_},

    {"copy", cmds::copy_cp_},
    {"cp", cmds::copy_cp_},

    {"move", cmds::move_mv_},
    {"mv", cmds::move_mv_},

    {"lock", cmds::lock_unlock_},
    {"unlock", cmds::lock_unlock_},

    {"password", cmds::password_passwd_},
    {"passwd", cmds::password_passwd_},

    {"history", cmds::history_},
    {"echo", cmds::echo_},
    {"help", cmds::help_},

    {"clear", cmds::clear_cls_},
    {"cls", cmds::clear_cls_},

    {"fetch", cmds::fetch_},

    {"pwd", cmds::pwd_},

    {"whoami", cmds::whoami_},

    {"date", cmds::date_},

    {"time", cmds::time_},

    {"du", cmds::du_},

    {"tree", cmds::tree_},

    {"find", cmds::find_},

    {"poweroff", cmds::poweroff_reboot_},
    {"reboot", cmds::poweroff_reboot_},
};
