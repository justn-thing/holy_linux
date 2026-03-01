#pragma once
#include "../ui/Messages.hpp"

inline int Execute(CommandParams &param, const bool startupConfigPhase = false) {
    const std::string arg = param.args.empty() ? "" : param.args[0];

    if (param.cmd == "cd") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (Node* target = getAbsolute(arg);
            target && target->type == "dir") {
            if (target->metadata.sudo && !param.sudo)
                alert(msg::not_sudo, stx::yellow);
            else
                FS::current = target;
        } else
            alert(msg::dir_not_found, stx::red, startupConfigPhase);
    }
    else if (param.cmd == "dir" || param.cmd == "ls") {
        if (arg.empty())
            displayDir(FS::current);
        else if (const Node* target = getAbsolute(arg))
            displayDir(target);
        else
            alert(msg::dir_not_found, stx::red, startupConfigPhase);
    }
    else if (param.cmd == "mkdir") {
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

        if (Node* target = getAbsolute(path);
            target && target->type == "dir")
            newChild(target, name, "dir");
        else
            alert(msg::invalid_path, stx::red, startupConfigPhase);
    }
    else if (param.cmd == "rmdir") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        bool recursive = false;
        for (const char flag : param.flags) {
            recursive |= flag == 'r';
        }

        if (const Node* target = getAbsolute(arg); !target)
            alert(msg::dir_not_found, stx::red);
        else if (target == FS::root)
            alert(msg::cant_remove_root, stx::yellow);
        else if (target == FS::current || (recursive && isAncestor(target, FS::current)))
            alert(msg::cant_remove_self, stx::yellow);
        else if (target->metadata.sudo && !param.sudo)
            alert(msg::not_sudo, stx::yellow);
        else if (target->type != "dir")
            alert(msg::rmdir_file, stx::yellow);
        else
            removeNode(target, recursive);
    }
    else if (param.cmd == "mkfile" || param.cmd == "touch") {
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
        if (name.find('.') != std::string::npos || name.empty() || type.empty()) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        if (Node* target = getAbsolute(path);
            target && target->type == "dir")
            newChild(target, name, type);
        else
            alert(msg::invalid_path, stx::red, startupConfigPhase);
    }
    else if (param.cmd == "rmfile" || param.cmd == "rm") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (Node* target = getAbsolute(arg); !target)
            alert(msg::file_not_found, stx::red);
        else if (target->type == "dir")
            alert(msg::rmfile_dir, stx::yellow);
        else if (target->metadata.sudo && !param.sudo)
            alert(msg::not_sudo, stx::yellow);
        else
            removeNode(target);
    }
    else if (param.cmd == "rename") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }
        if (param.args[1].find('/') != std::string::npos) {
            alert(msg::invalid_arg, stx::yellow);
            return 0;
        }

        if (Node* target = getAbsolute(arg); !target)
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

            if (name.find('.') != std::string::npos || name.empty() || type.empty()) {
                alert(msg::invalid_arg, stx::yellow);
                return 0;
            }
            
            if (getChild(target->parent, name, type))
                alert(msg::file_alr_exists, stx::yellow, startupConfigPhase);
            else {
                target->name = name;
                target->type = type;
            }
        }
    }
    else if (param.cmd == "write" || param.cmd == "wr" || param.cmd == "edit") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* target = getAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        bool allowed = false;
        for (std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
            const std::string& allowedType : allowedTypes)
            allowed |= target->type == allowedType;

        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        HolyVim(target, param.cmd);
    }
    else if (param.cmd == "read" || param.cmd == "cat") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* target = getAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        bool allowed = false;
        for (std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
             const std::string& allowedType : allowedTypes)
            allowed |= target->type == allowedType;

        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        std::cout << target->value << "\n";
    }
    else if (param.cmd == "execute" || param.cmd == "exec") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* target = getAbsolute(arg);
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
                CommandParams params = std::move(parseCommandLine(line));
                Execute(params, true);
            }
        } else if (target->type == "py") {
            std::ofstream fileout("ram/pythonExecutable.txt");
            fileout << target->value;
            fileout.close();

            #ifdef _WIN32
                system("py ram/pythonExecutable.txt");
            #else
                system("python3 ram/pythonExecutable.txt");
            #endif


            std::ofstream fileErase("ram/pythonExecutable.txt");
            fileErase.close();
        } else if (target->type == "exe") {
            #ifdef _WIN32
                std::ofstream fileout("ram/winExecutable.exe", std::ios::binary);
                fileout << target->value;
                fileout.close();

                system("cd ram && winExecutable.exe");
            #else
                std::ofstream fileout("ram/linuxExecutable", std::ios::binary);
                fileout << target->value;
                fileout.close();

                system("chmod +x ram/linuxExecutable && ram/linuxExecutable");
            #endif
        } else
            alert(msg::invalid_file_type, stx::yellow);
    }
    else if (param.cmd == "compile" || param.cmd == "comp") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        Node* target = getAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }
        if (target->metadata.sudo && !param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        if (target->type == "cpp") {
            std::ofstream fileout("ram/cppCompileable.cpp");
            fileout << target->value;
            fileout.close();

            #ifdef _WIN32
                const std::string compiledPath = "ram/cppCompiled.exe";
                const std::string compileCmd = "g++ ram/cppCompileable.cpp -o ram/cppCompiled.exe";
            #else
                const std::string compiledPath = "ram/cppCompiled";
                const std::string compileCmd = "g++ ram/cppCompileable.cpp -o ram/cppCompiled";
            #endif

            if (system(compileCmd.c_str()) != 0) {
                alert(msg::fail_compile, stx::red);
                return 0;
            }

            std::ofstream fileErase("ram/cppCompileable.cpp");
            fileErase.close();

            std::ifstream filein(compiledPath, std::ios::binary);
            if (!filein.is_open()) {
                alert(msg::fail_compile, stx::red);
                return 0;
            }
            auto output = std::string(std::istreambuf_iterator(filein), std::istreambuf_iterator<char>());
            filein.close();
            std::filesystem::remove(compiledPath);

            removeChild(target->parent, target->name, "exe");
            Node* exeFile = newChild(target->parent, target->name, "exe");
            exeFile->value = std::move(output);
        } else
            alert(msg::invalid_file_type, stx::yellow);
    }
    else if (param.cmd == "mount" || param.cmd == "mnt") {
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

        Node* mount = getChild(FS::root, "mnt", "dir");
        if (!mount) {
            alert(msg::mnt_doesnt_exist, stx::red);
            return 0;
        }

        bool allowed = false;
        for (std::array<std::string, 4> allowedTypes = {"txt", "cmd", "py", "cpp"};
            const std::string& allowedType : allowedTypes)
            allowed |= type == allowedType;

        if (!allowed) {
            alert(msg::invalid_file_type, stx::yellow);
            return 0;
        }

        if (Node* mountedFile = newChild(mount, name, type))
            mountedFile->value = fileValue.str();
        else
            alert(msg::file_alr_exists, stx::yellow);
    }
    else if (param.cmd == "copy" || param.cmd == "cp" ||
             param.cmd == "move" || param.cmd == "mv") {
        std::string targetStr;
        if (param.args.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (param.args.size() < 2) {
            targetStr = getPath(FS::current);
        } else
            targetStr = param.args[1];

        Node* fromNode = getAbsolute(arg);
        Node* toNode = getAbsolute(targetStr);
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

        if (Node* toChild = newChild(toNode, fromNode->name, fromNode->type, fromNode->metadata.sudo, fromNode->metadata.misc))
            toChild->value = fromNode->value;
        else {
            alert(msg::file_alr_exists, stx::yellow);
            return 0;
        }

        if (param.cmd == "move" || param.cmd == "mv") {
            removeNode(fromNode);
        }
    }
    else if (param.cmd == "lock" || param.cmd == "unlock") {
        if (arg.empty()) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (!param.sudo) {
            alert(msg::not_sudo, stx::yellow);
            return 0;
        }

        Node* target = getAbsolute(arg);
        if (!target) {
            alert(msg::file_not_found, stx::red);
            return 0;
        }

        lockNode(target, param.cmd == "lock");
    }
    else if (param.cmd == "password" || param.cmd == "passwd") {
        if (param.args.size() < 2) {
            alert(msg::arg_missing, stx::yellow);
            return 0;
        }

        if (param.args[0] == param.args[1] && !param.args[0].empty()) {
            if (!changePassword(SData::username, arg))
                alert(msg::pass_set_fail, stx::yellow);
        } else
            alert(msg::pass_set_fail, stx::yellow);
    }
    else if (param.cmd == "help") {
        std::cout << page::help << "\n";
    }
    else if (param.cmd == "clear" || param.cmd == "cls") {
        stx::clearConsole();
    }
    else if (param.cmd == "fetch") {
        std::cout << page::fetch << "\n";
    }
    else if (param.cmd == "pwd") {
        std::cout << getPath(FS::current) << "\n";
    }
    else if (param.cmd == "poweroff") {
        alert(msg::begin_poweroff, stx::green);

        bool discardChanges = false;
        for (const char flag : param.flags) {
            discardChanges |= flag == 'd';
        }

        if (discardChanges) return 99;

        alert(msg::begin_save_fs, stx::green);
        if (const bool saveSuccess = saveFileSystem();
            !saveSuccess)
            alert(msg::fail_save_filesystem, stx::red);

        return 99;
    }
    else if (!param.cmd.empty()) {
        alert(msg::unknown_cmd, stx::red);
    }
    return 0;
}
