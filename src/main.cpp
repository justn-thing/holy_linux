#include "Headers.hpp"

int main() {
    cout << stx::green << "Booting...\n" << stx::reset;

    const auto root = make_unique<Node>("dir", "root", "");
    Node* current = root.get();
    if (const bool loadFilesystemSuccess = loadFilesystem(current);
        !loadFilesystemSuccess)
        throwError(err::fail_load_filesystem, stx::red);

    size_t executionState = -1;
    string* executionValue = nullptr;

    bool startupConfigPhase = false;
    if (ifstream startupConfig("rom/startup.txt"); startupConfig.is_open()) {
        cout << stx::green << "Beginning startup config...\n" << stx::reset;
        stringstream ss;
        ss << startupConfig.rdbuf();
        static string startupBuffer = ss.str();

        executionState = 0;
        executionValue = &startupBuffer;
        startupConfigPhase = true;
    } else {
        throwError(err::fail_load_startupcfg, stx::red);
    }

    bool sudo;

    while (true) {
        string input, cmd, flags, arg;

        if (executionState == -1) {
            cout << stx::gray << "<" << getPath(current) << "> $" << stx::reset;
            getline(cin, input);
        } else {
            if (executionValue) {
                if (vector<string> commands = split(*executionValue, '\n');
                executionState < commands.size()) {
                    input = commands[executionState];
                    if (!startupConfigPhase) {
                        cout << input << "\n";
                    }
                    ++executionState;
                } else {
                    executionState = -1;
                    executionValue = nullptr;
                    if (startupConfigPhase) {
                        startupConfigPhase = false;
                        loadBootCfg(root.get());
                        cout << stx::green << "Boot sequence completed!\n" << stx::reset;
                        this_thread::sleep_for(chrono::milliseconds(500));
                        stx::clearConsole();
                        cout << page::fetch << "\n";
                    }
                }
            } else {
                throwError(err::exec_not_found, stx::red);
            }
        }

        stringstream ss(input);
        ss >> cmd;
        ss >> arg;

        sudo = false;

        if (cmd == "sudo") {
            cmd = arg;
            if (!(ss >> arg)) {
                arg.erase();
            }
            sudo = true;
        }

        if (arg.starts_with('-')) {
            flags = arg.substr(1);
            if (!(ss >> arg)) {
                arg.erase();
            }
        }

        if (cmd == "cd") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            if (arg == "..") {
                if (current->parent) {
                    current = current->parent;
                }
            }
            else if (arg.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), arg, "Directory");
                if (!target) {
                    throwError(err::dir_not_found, stx::red);
                    continue;
                }

                current = target;
            } else {
                if (Node* node = getChild(current, arg, "dir")) {
                    current = node;
                } else {
                    throwError(err::dir_not_found, stx::red);
                }
            }
        }
        else if (cmd == "dir" || cmd == "ls") {
            if (arg.empty()) {
                displayDir(current);
            }
            else if (arg.starts_with("/root")) {
                if (arg == "/root" || arg == "/root/") {
                    displayDir(root.get());
                    continue;
                }

                Node* target = getAbsolute(root.get(), arg, "Directory");
                if (!target) {
                    throwError(err::dir_not_found, stx::red);
                    continue;
                }

                displayDir(target);
            }
            else {
                if (Node* node = getChild(current, arg, "dir")) {
                    displayDir(node);
                } else {
                    throwError(err::dir_not_found, stx::red);
                }
            }
        }
        else if (cmd == "mkdir") {
            if (arg.find('/') != string::npos) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
            } else if (getChild(current, arg, "dir")) {
                throwError(err::dir_alr_exists, stx::yellow, startupConfigPhase);
            } else if (arg.find('.') != string::npos) {
                throwError(err::invalid_arg, stx::yellow);
            } else {
                newChild(current, arg, "dir");
            }
        }
        else if (cmd == "rmdir") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            bool recursive = false;
            for (char flag : flags) {
                recursive |= flag == 'r';
            }

            if (arg.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), arg, "Directory");
                if (!target) {
                    throwError(err::dir_not_found, stx::red);
                    continue;
                }
                if (target == root.get()) {
                    if (!sudo) {
                        throwError(err::not_sudo, stx::yellow);
                        continue;
                    }

                    cout << stx::red << "Are you sure? This may cause fatal corruption. (y/n)\n" << stx::reset;
                    string confirmation;
                    getline(cin, confirmation);
                    transform(confirmation.begin(), confirmation.end(), confirmation.begin(), ::tolower);

                    if (confirmation == "y" || confirmation == "yes") {
                        removeNode(target, true);
                        continue;
                    }
                    throwError(err::fail_confirm, stx::yellow);
                    continue;
                }
                if (target == current || (recursive && isAncestor(root.get(), target, current))) {
                    throwError(err::cant_remove_self, stx::yellow);
                    continue;
                }

                if (target->metadata.sudo && !sudo) {
                    throwError(err::not_sudo, stx::yellow);
                    continue;
                }

                removeNode(target, recursive);
            }
            else {
                Node* target = getChild(current, arg, "dir");
                if (!target) {
                    throwError(err::dir_not_found, stx::red);
                    continue;
                }
                if (target->metadata.sudo && !sudo) {
                    throwError(err::not_sudo, stx::yellow);
                    continue;
                }

                removeNode(target, recursive);
            }
        }
        else if (cmd == "mkfile" || cmd == "touch") {
            vector<string> file = split(arg, '.');

            if (arg.find('/') != string::npos) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            if (const size_t fileSize = file.size();
                fileSize == 1) {
                file.emplace_back("txt");
            } else if (fileSize != 2) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            if (file[1] == "dir") {
                throwError(err::mkfile_dir, stx::yellow);
                continue;
            }
            if (getChild(current, file[0], file[1])) {
                throwError(err::file_alr_exists, stx::yellow, startupConfigPhase);
                continue;
            }

            newChild(current, file[0], file[1]);
        }
        else if (cmd == "rmfile" || cmd == "rm") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            if (arg.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), arg, "File");
                if (!target) {
                    throwError(err::file_not_found, stx::red);
                    continue;
                }
                if (target->type == "dir") {
                    throwError(err::rmfile_dir, stx::yellow);
                    continue;
                }
                if (target->metadata.sudo && !sudo) {
                    throwError(err::not_sudo, stx::yellow);
                    continue;
                }

                removeNode(target);
            }
            else {
                vector<string> file = split(arg, '.');
                if (const size_t fileSize = file.size();
                    fileSize == 1) {
                    throwError(err::rmfile_dir, stx::yellow);
                    continue;
                } else if (fileSize != 2) {
                    throwError(err::invalid_arg, stx::yellow);
                    continue;
                }
                Node* target = getChild(current, file[0], file[1]);
                if (!target) {
                    throwError(err::file_not_found, stx::red);
                    continue;
                }
                if (target->metadata.sudo && !sudo) {
                    throwError(err::not_sudo, stx::yellow);
                    continue;
                }

                removeNode(target);
            }
        }
        else if (cmd == "rename") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            string name;
            if (!(ss >> name)) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }
            if (name.find('.') != string::npos) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            vector<string> file = split(split(arg, '/').back(), '.');
            if (file.size() == 1) {
                file.emplace_back("dir");
            }
            if (file.size() != 2) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            if (arg.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), arg, "File");
                if (!target) {
                    throwError(err::file_not_found, stx::red);
                    continue;
                }

                if (target == root.get()) {
                    if (!sudo) {
                        throwError(err::not_sudo, stx::yellow);
                        continue;
                    }

                    cout << stx::red << "Are you sure? This may cause fatal corruption. (y/n)\n" << stx::reset;
                    string confirmation;
                    getline(cin, confirmation);
                    transform(confirmation.begin(), confirmation.end(), confirmation.begin(), ::tolower);

                    if (confirmation == "y" || confirmation == "yes") {
                        removeNode(target, true);
                        continue;
                    }
                    throwError(err::fail_confirm, stx::yellow);
                    continue;
                }

                if (getChild(target->parent, name, file[1])) {
                    throwError(err::file_alr_exists, stx::yellow);
                    continue;
                }

                if (target->metadata.sudo && !sudo) {
                    throwError(err::not_sudo, stx::yellow);
                    continue;
                }

                target->name = name;
            } else {
                Node* target = getChild(current, file[0], file[1]);
                if (!target) {
                    throwError(err::file_not_found, stx::red);
                    continue;
                }
                if (getChild(current, name, file[1])) {
                    throwError(err::file_alr_exists, stx::yellow);
                    continue;
                }
                if (target->metadata.sudo && !sudo) {
                    throwError(err::not_sudo, stx::yellow);
                    continue;
                }

                target->name = name;
            }
        }
        else if (cmd == "write" || cmd == "wr" || cmd == "edit") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }
            vector<string> file = split(arg, '.');
            if (file.size() != 2) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }
            if (file[1] != "txt" && file[1] != "cmd" && file[1] != "py" && file[1] != "cpp") {
                throwError(err::invalid_file_type, stx::red);
                continue;
            }

            Node* target = getChild(current, file[0], file[1]);
            if (!target) {
                throwError(err::file_not_found, stx::red);
                continue;
            }
            if (target->metadata.sudo && !sudo) {
                throwError(err::not_sudo, stx::yellow);
                continue;
            }

            cout << page::holyVim << "\n";

            vector<string> output;
            size_t vimIndex = 1;

            if (cmd == "edit") {
                output = split(target->value, '\n', true);
                vimIndex = output.size() + 1;

                ostringstream buffer;
                for (auto prevIndex{1uz}; prevIndex < vimIndex; ++prevIndex) {
                    buffer << stx::gray << prevIndex;
                    if (prevIndex < 10) {
                        buffer << "  ";
                    } else if (prevIndex < 100) {
                        buffer << " ";
                    }
                    buffer << "> " << stx::reset << output[prevIndex - 1] << "\n";
                }
                cout << buffer.str();
            }

            while (true) {
                cout << stx::gray << vimIndex;
                if (vimIndex < 10) {
                    cout << "  ";
                } else if (vimIndex < 100) {
                    cout << " ";
                }
                cout << "> " << stx::reset;

                string temp;
                getline(cin, temp);

                if (const vector<string> cmds = split(temp, ' ', true, true);
                    cmds[0] == ":goto" || cmds[0] == ":ins" || cmds[0] == ":del") {
                    if (cmds.size() < 2) {
                        throwError(err::arg_missing, stx::yellow);
                        continue;
                    }
                    stringstream stringToSize_t(cmds[1]);

                    if (size_t convertedStr; stringToSize_t >> convertedStr) {
                        if (convertedStr > 0 && convertedStr <= output.size()) {
                            if (cmds[0] == ":ins") {
                                output.insert(output.begin() + static_cast<int>(convertedStr) - 1, "");
                            }
                            if (cmds[0] == ":del") {
                                output.erase(output.begin() + static_cast<int>(convertedStr) - 1);
                                --vimIndex;
                                continue;
                            }
                            vimIndex = convertedStr;
                        } else {
                            throwError(err::invalid_arg, stx::yellow);
                        }
                    } else {
                        throwError(err::invalid_arg, stx::yellow);
                    }
                    continue;
                }
                if (temp == ":save") {
                    string finalOutput;
                    for (const string& line : output) {
                        finalOutput += line + "\n";
                    }
                    target->value = finalOutput;
                    break;
                }
                if (temp == ":discard") {
                    break;
                }
                if (vimIndex > output.size()) {
                    output.push_back(temp);
                } else {
                    output[vimIndex - 1] = temp;
                }

                ++vimIndex;
            }
        }
        else if (cmd == "read" || cmd == "cat") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }
            vector<string> file = split(arg, '.');
            if (file.size() != 2) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }
            if (file[1] != "txt" && file[1] != "cmd" && file[1] != "py" && file[1] != "cpp") {
                throwError(err::invalid_file_type, stx::red);
                continue;
            }

            Node* target = getChild(current, file[0], file[1]);
            if (!target) {
                throwError(err::file_not_found, stx::red);
                continue;
            }

            cout << target->value << "\n";
        }
        else if (cmd == "execute" || cmd == "exec") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }
            vector<string> file = split(arg, '.');
            if (file.size() != 2) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            Node* target = getChild(current, file[0], file[1]);
            if (!target) {
                throwError(err::file_not_found, stx::red);
                continue;
            }
            if (target->metadata.sudo && !sudo) {
                throwError(err::not_sudo, stx::yellow);
                continue;
            }

            if (file[1] == "cmd") {
                executionState = 0;
                executionValue = &target->value;
            } else if (file[1] == "py") {
                ofstream fileout("ram/pythonExecutable.txt");
                fileout << target->value;
                fileout.close();

                #ifdef _WIN32
                    system("py ram/pythonExecutable.txt");
                #else
                    system("python3 ram/pythonExecutable.txt");
                #endif


                ofstream fileErase("ram/pythonExecutable.txt");
                fileErase.close();
            } else if (file[1] == "exe") {
                #ifdef _WIN32
                    ofstream fileout("ram/winExecutable.exe", ios::binary);
                    fileout << target->value;
                    fileout.close();

                    system("cd ram && winExecutable.exe");
                #else
                    ofstream fileout("ram/linuxExecutable", ios::binary);
                    fileout << target->value;
                    fileout.close();

                    system("chmod +x ram/linuxExecutable");
                    system("cd ram && ./linuxExecutable");
                #endif
            } else {
                throwError(err::invalid_file_type, stx::red);
            }
        }
        else if (cmd == "compile" || cmd == "comp") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }
            vector<string> file = split(arg, '.');
            if (file.size() != 2) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            Node* target = getChild(current, file[0], file[1]);
            if (!target) {
                throwError(err::file_not_found, stx::red);
                continue;
            }
            if (target->metadata.sudo && !sudo) {
                throwError(err::not_sudo, stx::yellow);
                continue;
            }

            if (file[1] == "cpp") {
                ofstream fileout("ram/cppCompileable.cpp");
                fileout << target->value;
                fileout.close();

                #ifdef _WIN32
                    const string compiledPath = "ram/cppCompiled.exe";
                    const string compileCmd = "g++ ram/cppCompileable.cpp -o ram/cppCompiled.exe";
                #else
                    const string compiledPath = "ram/cppCompiled";
                    const string compileCmd = "g++ ram/cppCompileable.cpp -o ram/cppCompiled";
                #endif

                if (system(compileCmd.c_str()) != 0) {
                    throwError(err::fail_compile, stx::red);
                    continue;
                }

                ofstream fileErase("ram/cppCompileable.cpp");
                fileErase.close();

                ifstream filein(compiledPath, ios::binary);
                auto output = string(istreambuf_iterator(filein), istreambuf_iterator<char>());
                filein.close();
                filesystem::remove(compiledPath);

                if (getChild(target->parent, target->name, "exe")) {
                    removeChild(target->parent, target->name, "exe");
                }
                Node* exeFile = newChild(target->parent, target->name, "exe");
                exeFile->value = std::move(output);
            } else {
                throwError(err::invalid_file_type, stx::red);
            }
        }
        else if (cmd == "mount" || cmd == "mnt") {
            if (!sudo) {
                throwError(err::not_sudo, stx::yellow);
                continue;
            }

            string name;
            if (!(ss >> name)) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            ifstream filein(arg);
            if (!filein.is_open()) {
                throwError(err::fail_mount, stx::red);
                continue;
            }
            stringstream fileValue;
            fileValue << filein.rdbuf();

            Node* mount = getChild(root.get(), "mnt", "dir");
            if (!mount) {
                throwError(err::mnt_doesnt_exist, stx::red);
                continue;
            }

            if (const vector<string> file = split(arg, '.');
                file.back() == "txt" || file.back() == "cmd" || file.back() == "py" || file.back() == "cpp") {
                if (!getChild(mount, name, file.back())) {
                    Node* mountedFile = newChild(mount, name, file.back());
                    mountedFile->value = fileValue.str();
                } else {
                    throwError(err::file_alr_exists, stx::yellow);
                }
            } else {
                throwError(err::unknown_file_type, stx::red);
            }
        }
        else if (cmd == "copy" || cmd == "move" || cmd == "mov") {
            string targetStr;
            if (!(ss >> targetStr)) {
                targetStr = getPath(current);
            }

            const vector<string> from = split(arg, '.');
            const vector<string> to = split(targetStr, '.');
            if (from.size() != 2 || to.size() != 1 || split(from[1], '/').size() != 1) {
                throwError(err::invalid_arg, stx::yellow);
                continue;
            }

            Node* fromNode;

            if (arg.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), arg, "File");
                if (!target) continue;

                fromNode = target;
            }
            else {
                Node* target = getChild(current, from[0], from[1]);
                if (!target) {
                    throwError(err::file_not_found, stx::red);
                    continue;
                }
                fromNode = target;
            }

            if (fromNode->metadata.sudo && !sudo) {
                throwError(err::not_sudo, stx::yellow);
                continue;
            }

            const string newNodeName = split(from[0], '/').back();

            if (targetStr.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), targetStr, "Directory");
                if (!target) continue;

                if (getChild(target, newNodeName, from[1])) {
                    throwError(err::file_alr_exists, stx::yellow);
                    continue;
                }

                Node* newNode = newChild(target, newNodeName, from[1]);
                newNode->value = fromNode->value;
                newNode->metadata = fromNode->metadata;
            }
            else {
                Node* target = getChild(current, to[0], "dir");
                if (!target) {
                    throwError(err::dir_not_found, stx::red);
                    continue;
                }

                if (getChild(target, newNodeName, from[1])) {
                    throwError(err::file_alr_exists, stx::yellow);
                    continue;
                }

                Node* newNode = newChild(target, newNodeName, from[1]);
                newNode->value = fromNode->value;
                newNode->metadata = fromNode->metadata;
            }

            if (cmd == "move" || cmd == "mov") {
                removeNode(fromNode);
            }
        }
        else if (cmd == "lock" || cmd == "unlock") {
            if (arg.empty()) {
                throwError(err::arg_missing, stx::yellow);
                continue;
            }

            if (!sudo) {
                throwError(err::not_sudo, stx::yellow);
                continue;
            }

            if (arg.starts_with("/root")) {
                Node* target = getAbsolute(root.get(), arg, "File");
                if (!target) continue;

                lockNode(target, cmd == "lock");
            }
            else {
                vector<string> file = split(arg, '.');
                if (file.size() == 1) {
                    file.emplace_back("dir");
                } else if (file.size() != 2) {
                    throwError(err::invalid_arg, stx::yellow);
                    continue;
                }

                Node* target = getChild(current, file[0], file[1]);
                if (!target) continue;

                lockNode(target, cmd == "lock");
            }
        }
        else if (cmd == "help") {
            cout << page::help << "\n";
        }
        else if (cmd == "clear" || cmd == "cls") {
            stx::clearConsole();
        }
        else if (cmd == "fetch") {
            cout << page::fetch << "\n";
        }
        else if (cmd == "pwd") {
            cout << getPath(current) << "\n";
        }
        else if (cmd == "poweroff") {
            cout << "System shutdown in progress.\n";

            bool discardChanges = false;
            for (const char flag : flags) {
                discardChanges |= flag == 'd';
            }

            if (discardChanges) break;

            cout << stx::green << "Saving filesystem..." << stx::reset << "\n";
            if (const bool saveSuccess = saveFilesystem(root.get());
                !saveSuccess)
                throwError(err::fail_save_filesystem, stx::red);

            cout << stx::green << "Saving startup config..." << stx::reset << "\n";
            const Node* boot = getChild(root.get(), "boot", "dir");
            if (!boot) {
                throwError(err::fail_save_startupcfg, stx::red);
                break;
            }

            const Node* config = getChild(boot, "startupConfig", "cmd");
            if (!config) {
                throwError(err::fail_save_startupcfg, stx::red);
                break;
            }

            ofstream fileout("rom/startup.txt");
            if (!fileout.is_open()) {
                throwError(err::fail_save_startupcfg, stx::red);
                break;
            }
            fileout << config->value;
            fileout.close();
            break;
        }
        else if (!cmd.empty()) {
            throwError(err::unknown_cmd, stx::red);
        }
    }

    return 0;
}
