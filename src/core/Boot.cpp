#include "Boot.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "../cmd/CmdParser.hpp"
#include "../cmd/Execution.hpp"
#include "../core/Run.hpp"
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
    void InitFileSystem() {
        alert(msg::begin_load_fs, stx::green);

        if (!LoadFileSystem()) {
            alert(msg::fail_load_filesystem, stx::red);
        }
    }

    int InitStartupConfig() {
        alert(msg::begin_startupcfg, stx::green);

        if (const Node* startupConfig = GetAbsolute("/boot/startupConfig.cmd")) {
            for (const std::string& line : split(startupConfig->_value, '\n')) {
                CmdParams params = ParseCommandLine(line);
                ExecuteCmdLine(params); // ignores poweroff and reboot
            }
        } else {
            alert(msg::startupcfg_not_exist, stx::yellow);
            Node* boot;
            if (!(boot = FS::root->GetAbsolute("boot"))) {
                boot = FS::root->NewChild("boot", "dir");
            }
            if (!FS::root->GetAbsolute("boot/startupConfig.cmd")) {
                boot->NewChild("startupConfig", "cmd");
            }
        }

        return 0;
    }

    void CreateMissingFile(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::ofstream fileout(path);
            fileout.close();
        }
    }

#ifdef _WIN32
    std::filesystem::path GetExecutablePath(char* args[]) {
        std::wstring buffer(MAX_PATH, L'\0');

        while (true) {
            const DWORD size = GetModuleFileNameW(nullptr, buffer.data(), buffer.size());
            if (size == 0) {
                return std::filesystem::absolute(args[0]);
            }

            if (size < buffer.size()) {
                buffer.resize(size);
                return std::filesystem::path{buffer};
            }

            buffer.resize(buffer.size() * 2);
        }
    }
#else
    std::filesystem::path GetExecutablePath(char* args[]) {
        return std::filesystem::absolute(args[0]);
    }
#endif

    void InitFileDependencies() {
        using namespace std::filesystem;

        const path ramDir = SData::ExternFS::selfParentPath / "ram";
        SData::ExternFS::RAM::cpp = ramDir / "cppCompilable.cpp";
        SData::ExternFS::RAM::py = ramDir / "pythonExecutable.txt";
#ifdef _WIN32
        SData::ExternFS::RAM::exe = ramDir / "winExecutable.exe";
#else
        SData::RAM::exe = ramDir / "linuxExecutable";
#endif

        const path romDir = SData::ExternFS::selfParentPath / "rom";
        SData::ExternFS::ROM::fileSystem = romDir / "fileSystem.txt";

        SData::ExternFS::Export::selfDir = SData::ExternFS::selfParentPath / "export";

        create_directory(ramDir);
        create_directory(romDir);
        create_directory(SData::ExternFS::Export::selfDir);

        CreateMissingFile(SData::ExternFS::RAM::cpp);
        CreateMissingFile(SData::ExternFS::RAM::py);
        CreateMissingFile(SData::ExternFS::RAM::exe);

        CreateMissingFile(SData::ExternFS::ROM::fileSystem);
    }

    int Boot() {
        alert(msg::begin_boot, stx::green);

        InitFileDependencies();

        InitFileSystem();

        InitStartupConfig();

        SData::user.root = false;
        stx::ClearConsole();

        if (Login() == -1) return -1;

        stx::ClearConsole();
        std::cout << page::fetch << '\n';

        return Run();
    }
}

int InitSystem(char* args[]) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    SData::ExternFS::selfParentPath = GetExecutablePath(args).parent_path();

    while (true) {
        if (const int& returnCode = Boot(); returnCode != Reboot) {
            return returnCode;
        }

        SData::user = CurrentUser {};
        FS::Reset();
        stx::ClearConsole();
    }
}
