#include "Execution.hpp"

#include <array>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>

#include "../apps/HolyVim.hpp"
#include "../cmd/CommandParser.hpp"
#include "../core/ReturnCodes.hpp"
#include "../fs/FileSaving.hpp"
#include "../fs/FileTree.hpp"
#include "../session/Login.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Pages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

int Execute(CommandParams& param, const bool startupConfigPhase) {
    if (param.cmd == "cd") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (Node* target = GetAbsolute(param.args[0]);
            target && target->type == "dir") {
            if (target->metadata.sudo && !param.sudo)
                alert(msg::not_sudo, stx::yellow);
            else
                FS::current = target;
        } else
            alert(msg::dir_not_found, stx::red, startupConfigPhase);
    } else if (param.cmd == "dir" || param.cmd == "ls") {
        if (param.args.empty())
            DisplayDir(FS::current);
        else if (const Node* target = GetAbsolute(param.args[0]))
            DisplayDir(target);
        else
            alert(msg::dir_not_found, stx::red, startupConfigPhase);
    } else if (param.cmd == "mkdir") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const size_t index = param.args[0].rfind('/');
        std::string path;
        std::string name;

        if (index == std::string::npos)
            name = param.args[0];
        else if (index == param.args[0].size() - 1) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        } else {
            path = param.args[0].substr(0, index + 1);
            name = param.args[0].substr(index + 1);
        }

        if (name.contains('.') || name.empty()) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        if (Node* target = GetAbsolute(path);
            target && target->type == "dir")
            NewChild(target, name, "dir");
        else
            alert(msg::invalid_path, stx::red, startupConfigPhase);
    } else if (param.cmd == "rmdir") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        bool recursive = false;
        for (const char& flag : param.shortFlags) {
            if (flag == 'r') recursive = true;
            else {
                alert(msg::unknown_flag, stx::yellow);
                return 0;
            }
        }

        if (const Node* target = GetAbsolute(param.args[0]); !target)
            alert(msg::dir_not_found, stx::red);
        else if (target == FS::root)
            alert(msg::cant_remove_root, stx::yellow);
        else if (target == FS::current || (recursive && IsAncestor(target, FS::current)))
            alert(msg::cant_remove_self, stx::yellow);
        else if (ContainsLockedNode(target) && !param.sudo)
            alert(msg::not_sudo, stx::yellow);
        else if (target->type != "dir")
            alert(msg::rmdir_file, stx::yellow);
        else
            RemoveNode(target, recursive);
    } else if (param.cmd == "mkfile" || param.cmd == "touch") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const size_t index = param.args[0].rfind('/');
        std::string path;
        std::string name;
        std::string type;

        if (index == std::string::npos)
            name = param.args[0];
        else {
            path = param.args[0].substr(0, index + 1);
            name = param.args[0].substr(index + 1);
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
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (const Node* target = GetAbsolute(param.args[0]); !target)
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

        if (Node* target = GetAbsolute(param.args[0]); !target)
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
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* target = GetAbsolute(param.args[0]);
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
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* target = GetAbsolute(param.args[0]);
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
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* target = GetAbsolute(param.args[0]);
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
                CommandParams params = ParseCommandLine(line);
                if (const int& returnCode = Execute(params, true); returnCode != 0)
                    return returnCode;
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
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* target = GetAbsolute(param.args[0]);
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
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const std::string fileName = std::filesystem::path(param.args[0]).filename().string();
        const size_t index = fileName.rfind('.');
        if (index == std::string::npos) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        const std::string name = fileName.substr(0, index);
        const std::string type = fileName.substr(index + 1);
        if (name.contains('.') || name.empty() || type.empty()) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        bool allowed = false;
        for (const std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
            const std::string& allowedType : allowedTypes) {
            if (type == allowedType) {
                allowed = true;
                break;
            }
        }
        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        std::ifstream filein(param.args[0], std::ios::binary);
        if (!filein.is_open()) {
            alert(msg::fail_mount, stx::red);
            return 0;
        }

        Node* mount = GetChild(FS::root, "mnt", "dir");
        if (!mount)
            mount = NewChild(FS::root, "mnt", "dir", true);

        if (Node* mountedFile = NewChild(mount, name, type)) {
            filein.seekg(0, std::ios::end);
            std::string data;
            data.resize(filein.tellg());

            filein.seekg(0, std::ios::beg);
            filein.read(data.data(), static_cast<std::streamsize>(data.size()));

            mountedFile->value = std::move(data);
        } else alert(msg::file_alr_exists, stx::yellow);
    } else if (param.cmd == "export") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }
        if (!param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        Node* target = GetAbsolute(param.args[0]);
        if (!target) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }
        if (target->type == "dir") {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        const std::filesystem::path newFile = SData::Export::selfDir / (target->name + '.' + target->type);

        std::ofstream fileout(newFile, std::ios::binary);
        if (!fileout.is_open()) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }

        fileout.write(target->value.data(), static_cast<std::streamsize>(target->value.size()));
    } else if (param.cmd == "copy" || param.cmd == "cp") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* fromNode = GetAbsolute(param.args[0]);
        if (!fromNode) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }

        Node* toNode = GetAbsolute(param.args[1]);
        if (!toNode) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }
        if (toNode->type != "dir") {
            alert(msg::copy_into_file, stx::yellow);
            return 0;
        }

        if (IsAncestor(fromNode, toNode)) {
            alert(msg::copy_into_descendant, stx::yellow);
            return 0;
        }

        CopyNode(fromNode, toNode, param.sudo);
    } else if (param.cmd == "move" || param.cmd == "mv") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        const Node* fromNode = GetAbsolute(param.args[0]);
        if (!fromNode) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }
        if (fromNode == FS::root) {
            alert(msg::cant_move_root, stx::yellow);
            return 0;
        }

        Node* toNode = GetAbsolute(param.args[1]);
        if (!toNode) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }
        if (toNode->type != "dir") {
            alert(msg::move_into_file, stx::yellow);
            return 0;
        }

        if (IsAncestor(fromNode, toNode)) {
            alert(msg::move_into_descendant, stx::yellow);
            return 0;
        }

        MoveNode(fromNode, toNode, param.sudo);
    } else if (param.cmd == "lock" || param.cmd == "unlock") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (!param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        Node* target = GetAbsolute(param.args[0]);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        bool recursive = false;
        for (const char& flag : param.shortFlags) {
            if (flag == 'r') recursive = true;
            else {
                alert(msg::unknown_flag, stx::yellow);
                return 0;
            }
        }

        LockNode(target, param.cmd == "lock", recursive);
    } else if (param.cmd == "password" || param.cmd == "passwd") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        bool root = false;
        for (const std::string& flag : param.longFlags) {
            if (flag == "root") {
                if (param.sudo) root = true;
                else {
                    alert(msg::not_sudo, stx::yellow);
                    return 0;
                }
            }
            else {
                alert(msg::unknown_flag, stx::yellow);
                return 0;
            }
        }

        if (param.args[0].empty() || param.args[0] != param.args[1])
            alert(msg::pass_set_fail, stx::yellow);
        else
            ChangePassword(root ? "//root//" : SData::username, param.args[0]);
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
    } else if (param.cmd == "date") {
        const auto now = std::chrono::system_clock::now();
        const std::time_t date = std::chrono::system_clock::to_time_t(now);

        std::cout << std::put_time(std::localtime(&date), "%d/%m/%Y") << "\n";
    } else if (param.cmd == "time") {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::cout << std::put_time(std::localtime(&time), "%H:%M:%S") << "\n";
    } else if (param.cmd == "du") {
        const Node* node;
        if (param.args.empty()) {
            node = FS::current;
        } else {
            node = GetAbsolute(param.args[0]);

            if (!node) {
                alert(msg::invalid_path, stx::red);
                return 0;
            }
        }

        size_t bytes = GetFileSize(node);
        double kB = static_cast<double>(bytes) / 1024.0;
        double MB = kB / 1024.0;
        double GB = MB / 1024.0;
        std::cout << std::format("{:.2f} GiB / {:.2f} MiB / {:.2f} KiB / {} B", GB, MB, kB, bytes) << "\n";
    } else if (param.cmd == "tree") {
        if (param.args.empty()) {
            PrintTree(FS::current);
            return 0;
        }

        if (const Node* target = GetAbsolute(param.args[0]))
            PrintTree(target);
        else
            alert(msg::invalid_path, stx::red);
    } else if (param.cmd == "find") {
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* ancestor;
        if (param.args.size() == 1) {
            ancestor = FS::current;
        } else {
            ancestor = GetAbsolute(param.args[1]);
            if (!ancestor) {
                alert(msg::invalid_path, stx::yellow);
                return 0;
            }
        }

        std::vector<Node*> result;
        if (const size_t dot = param.args[0].rfind('.');
            dot == std::string::npos || dot == param.args[0].size() - 1)
            FindNodes(ancestor, param.args[0].substr(0, dot), "", result, true);
        else
            FindNodes(ancestor, param.args[0].substr(0, dot), param.args[0].substr(dot + 1), result);

        if (result.empty()) {
            alert(msg::file_not_found, stx::yellow);
            return 0;
        }

        for (const Node* node : result) {
            std::cout << GetPath(node) << "\n";
        }
    } else if (param.cmd == "poweroff" || param.cmd == "reboot") {
        alert(msg::begin_poweroff, stx::green);

        const int returnCode = param.cmd == "poweroff" ? Poweroff : Reboot;

        bool discardChanges = false;
        for (const char& flag : param.shortFlags) {
            if (flag == 'd') discardChanges = true;
            else {
                alert(msg::unknown_flag, stx::yellow);
                return 0;
            }
        }

        if (discardChanges) return returnCode;

        alert(msg::begin_save_fs, stx::green);
        if (const bool saveSuccess = SaveFileSystem();
            !saveSuccess)
            alert(msg::fail_save_filesystem, stx::red);

        return returnCode;
    } else if (!param.cmd.empty()) {
        const Node* bin = GetChild(FS::root, "bin", "dir");
        if (!bin)
            bin = NewChild(FS::root, "bin", "dir");

        const std::string exec = param.sudo ? "sudo exec /bin/" : "exec /bin/";

        if (GetChild(bin, param.cmd, "exe")) {
            CommandParams line = ParseCommandLine(exec + param.cmd + ".exe");
            return Execute(line, startupConfigPhase);
        } if (GetChild(bin, param.cmd, "cmd")) {
            CommandParams line = ParseCommandLine(exec + param.cmd + ".cmd");
            return Execute(line, startupConfigPhase);
        } if (GetChild(bin, param.cmd, "py")) {
            CommandParams line = ParseCommandLine(exec + param.cmd + ".py");
            return Execute(line, startupConfigPhase);
        }

        alert(msg::unknown_cmd, stx::yellow);
    }
    return 0;
}
