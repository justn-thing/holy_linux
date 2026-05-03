#include "Execution.hpp"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "../apps/HolyVim.hpp"
#include "../cmd/CommandParser.hpp"
#include "../fs/FileSaving.hpp"
#include "../fs/FileTree.hpp"
#include "../session/Login.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Pages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

int Execute(CommandParams& param, const bool startupConfigPhase) {
    const std::string arg = param.args.empty() ? "" : param.args[0];

    if (param.cmd == "cd") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (Node* target = GetAbsolute(arg);
            target && target->type == "dir") {
            if (target->metadata.sudo && !param.sudo)
                alert(msg::not_sudo, stx::yellow);
            else
                FS::current = target;
        } else
            alert(msg::dir_not_found, stx::red, startupConfigPhase);
    } else if (param.cmd == "dir" || param.cmd == "ls") {
        if (arg.empty())
            DisplayDir(FS::current);
        else if (const Node* target = GetAbsolute(arg))
            DisplayDir(target);
        else
            alert(msg::dir_not_found, stx::red, startupConfigPhase);
    } else if (param.cmd == "mkdir") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const size_t index = arg.rfind('/');
        std::string path;
        std::string name;

        if (index == std::string::npos)
            name = arg;
        else if (const size_t dotIndex = arg.rfind('.');
                index == arg.size() - 1 || (dotIndex != std::string::npos && dotIndex > index)) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        } else {
            path = arg.substr(0, index + 1);
            name = arg.substr(index + 1);
        }

        if (Node* target = GetAbsolute(path);
            target && target->type == "dir")
            NewChild(target, name, "dir");
        else
            alert(msg::invalid_path, stx::red, startupConfigPhase);
    } else if (param.cmd == "rmdir") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        bool recursive = false;
        for (const char flag : param.flags) {
            recursive |= flag == 'r';
        }

        if (const Node* target = GetAbsolute(arg); !target)
            alert(msg::dir_not_found, stx::red);
        else if (target == FS::root)
            alert(msg::cant_remove_root, stx::yellow);
        else if (target == FS::current || (recursive && IsAncestor(target, FS::current)))
            alert(msg::cant_remove_self, stx::yellow);
        else if (target->metadata.sudo && !param.sudo)
            alert(msg::not_sudo, stx::yellow);
        else if (target->type != "dir")
            alert(msg::rmdir_file, stx::yellow);
        else
            RemoveNode(target, recursive);
    } else if (param.cmd == "mkfile" || param.cmd == "touch") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const size_t index = arg.rfind('/');
        std::string path;
        std::string name;
        std::string type;

        if (index == std::string::npos)
            name = arg;
        else {
            path = arg.substr(0, index + 1);
            name = arg.substr(index + 1);
        }

        if (const size_t dotIndex = name.rfind('.');
            dotIndex == std::string::npos)
            type = "txt";
        else {
            type = name.substr(dotIndex + 1);
            name = name.substr(0, dotIndex);
        }
        if (name.contains('.') || name.empty() || type.empty()) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        if (Node* target = GetAbsolute(path);
            target && target->type == "dir")
            NewChild(target, name, type);
        else
            alert(msg::invalid_path, stx::red, startupConfigPhase);
    } else if (param.cmd == "rmfile" || param.cmd == "rm") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (const Node* target = GetAbsolute(arg); !target)
            alert(msg::file_not_found, stx::red);
        else if (target->type == "dir")
            alert(msg::rmfile_dir, stx::yellow);
        else if (target->metadata.sudo && !param.sudo)
            alert(msg::not_sudo, stx::yellow);
        else
            RemoveNode(target);
    } else if (param.cmd == "rename") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }
        if (param.args[1].contains('/')) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        if (Node* target = GetAbsolute(arg); !target)
            alert(msg::file_not_found, stx::red);
        else if (target == FS::root)
            alert(msg::cant_rename_root, stx::yellow);
        else if (target->metadata.sudo && !param.sudo)
            alert(msg::not_sudo, stx::yellow);
        else {
            const size_t index = param.args[1].rfind('.');
            std::string name;
            std::string type;

            if (target->type == "dir" && index != std::string::npos) {
                alert(msg::invalid_file_type, stx::yellow);
                return 0;
            }

            if (index == std::string::npos) {
                name = param.args[1];
                type = target->type;
            } else {
                name = param.args[1].substr(0, index);
                type = param.args[1].substr(index + 1);
                if (type == "dir") {
                    alert(msg::invalid_file_type, stx::yellow);
                    return 0;
                }
            }

            if (name.contains('.') || name.empty() || type.empty()) {
                alert(msg::invalid_arg, stx::yellow);
                return 0;
            }

            if (GetChild(target->parent, name, type))
                alert(msg::file_alr_exists, stx::yellow, startupConfigPhase);
            else {
                target->name = name;
                target->type = type;
            }
        }
    } else if (param.cmd == "write" || param.cmd == "wr" || param.cmd == "edit") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* target = GetAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        bool allowed = false;
        for (const std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
            const std::string& allowedType : allowedTypes)
            allowed |= target->type == allowedType;

        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        HolyVim(target, param.cmd);
    } else if (param.cmd == "read" || param.cmd == "cat") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* target = GetAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        bool allowed = false;
        for (const std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
             const std::string& allowedType : allowedTypes)
            allowed |= target->type == allowedType;

        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        std::cout << target->value << "\n";
    } else if (param.cmd == "execute" || param.cmd == "exec") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* target = GetAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }
        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        if (target->type == "cmd") {
            for (const std::string& line : split(target->value, '\n')) {
                CommandParams params = parseCommandLine(line);
                Execute(params, true);
            }
        } else if (target->type == "py") {
            std::ofstream fileout(SData::RAM::py);
            fileout << target->value;
            fileout.close();

#ifdef _WIN32
            const std::wstring runCmd = L"py \"" + SData::RAM::py.wstring() + L"\"";
            _wsystem(runCmd.c_str());
#else
            const std::string runCmd = "python3 \"" + SData::RAM::py.string() + "\"";
            system(runCmd.c_str());
#endif

            std::ofstream fileErase(SData::RAM::py);
            fileErase.close();
        } else if (target->type == "exe") {
            std::ofstream fileout(SData::RAM::exe, std::ios::binary);
            fileout << target->value;
            fileout.close();
#ifdef _WIN32
            const std::wstring runCmd = L"\"" + SData::RAM::exe.wstring() + L"\"";
            _wsystem(runCmd.c_str());
#else
            const std::string runCmd = "chmod +x \"" + SData::RAM::exe.string() + "\" && \"" +
                                       SData::RAM::exe.string() + "\"";
            system(runCmd.c_str());
#endif
        } else
            alert(msg::invalid_file_type, stx::yellow);
    } else if (param.cmd == "compile" || param.cmd == "comp") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* target = GetAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }
        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        if (target->type == "cpp") {
            std::ofstream fileout(SData::RAM::cpp);
            fileout << target->value;
            fileout.close();

#ifdef _WIN32
            const std::filesystem::path compiledPath = SData::RAM::cpp.parent_path() / "cppCompiled.exe";
            const std::wstring compileCmd = L"g++ \"" + SData::RAM::cpp.wstring() + L"\" -o \"" +
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
                return 0;
            }

            std::ofstream fileErase(SData::RAM::cpp);
            fileErase.close();

            std::ifstream filein(compiledPath, std::ios::binary);
            if (!filein.is_open()) {
                alert(msg::fail_compile, stx::red);
                return 0;
            }
            auto output = std::string(std::istreambuf_iterator(filein), std::istreambuf_iterator<char>());
            filein.close();
            std::filesystem::remove(compiledPath);

            RemoveChild(target->parent, target->name, "exe");
            Node* exeFile = NewChild(target->parent, target->name, "exe");
            exeFile->value = std::move(output);
        } else
            alert(msg::invalid_file_type, stx::yellow);
    } else if (param.cmd == "mount" || param.cmd == "mnt") {
        if (!param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const size_t index = param.args[1].rfind('.');
        if (index == std::string::npos) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        const std::string name = param.args[1].substr(0, index);
        const std::string type = param.args[1].substr(index + 1);

        std::ifstream filein(arg);
        if (!filein.is_open()) {
            alert(msg::fail_mount, stx::red);
            return 0;
        }
        std::stringstream fileValue;
        fileValue << filein.rdbuf();

        Node* mount = GetChild(FS::root, "mnt", "dir");
        if (!mount) {
            alert(msg::mnt_doesnt_exist, stx::yellow);
            mount = NewChild(FS::root, "mnt", "dir", true);
        }

        bool allowed = false;
        for (const std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
            const std::string& allowedType : allowedTypes)
            allowed |= type == allowedType;

        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        if (Node* mountedFile = NewChild(mount, name, type))
            mountedFile->value = fileValue.str();
        else
            alert(msg::file_alr_exists, stx::yellow);
    } else if (param.cmd == "copy" || param.cmd == "cp" ||
               param.cmd == "move" || param.cmd == "mv") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        std::string targetStr;
        if (param.args.size() < 2) {
            targetStr = GetPath(FS::current);
        } else
            targetStr = param.args[1];

        const Node* fromNode = GetAbsolute(arg);
        Node* toNode = GetAbsolute(targetStr);
        if (!fromNode || !toNode) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        if (fromNode->parent == toNode)
            return 0;

        if (fromNode->type == "dir" || toNode->type != "dir") {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        if ((fromNode->metadata.sudo || toNode->metadata.sudo) && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        if (Node* toChild = NewChild(toNode, fromNode->name, fromNode->type, fromNode->metadata.sudo,
                                     fromNode->metadata.misc))
            toChild->value = fromNode->value;
        else {
            alert(msg::file_alr_exists, stx::yellow);
            return 0;
        }

        if (param.cmd == "move" || param.cmd == "mv") {
            RemoveNode(fromNode);
        }
    } else if (param.cmd == "lock" || param.cmd == "unlock") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (!param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        Node* target = GetAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        LockNode(target, param.cmd == "lock");
    } else if (param.cmd == "password" || param.cmd == "passwd") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (param.args[0] == param.args[1] && !param.args[0].empty()) {
            if (!ChangePassword(SData::username, arg))
                alert(msg::pass_set_fail, stx::yellow);
        } else
            alert(msg::pass_set_fail, stx::yellow);
    } else if (param.cmd == "history") {
        for (const std::string& cmd : SData::cmdHistory) {
            std::cout << cmd << "\n";
        }
    } else if (param.cmd == "echo") {
        for (const std::string& word : param.args)
            std::cout << word << " ";
        std::cout << "\n";
    } else if (param.cmd == "help") {
        std::cout << page::help << "\n";
    } else if (param.cmd == "clear" || param.cmd == "cls") {
        stx::ClearConsole();
    } else if (param.cmd == "fetch") {
        std::cout << page::fetch << "\n";
    } else if (param.cmd == "pwd") {
        std::cout << GetPath(FS::current) << "\n";
    } else if (param.cmd == "whoami") {
        std::cout << SData::username << "\n";
    } else if (param.cmd == "poweroff") {
        alert(msg::begin_poweroff, stx::green);

        bool discardChanges = false;
        for (const char flag : param.flags) {
            discardChanges |= flag == 'd';
        }

        if (discardChanges) return 99;

        alert(msg::begin_save_fs, stx::green);
        if (const bool saveSuccess = SaveFileSystem();
            !saveSuccess)
            alert(msg::fail_save_filesystem, stx::red);

        return 99;
    } else if (!param.cmd.empty()) {
        alert(msg::unknown_cmd, stx::yellow);
    }
    return 0;
}
