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

#include "../cmd/CommandParser.hpp"
#include "../cmd/Execution.hpp"
#include "../core/ReturnCodes.hpp"
#include "../core/Run.hpp"
#include "../fs/FileSaving.hpp"
#include "../fs/FileTree.hpp"
#include "../session/Login.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Pages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"

void BootFileSystem() {
    alert(msg::begin_load_fs, stx::green);

    if (!LoadFileSystem())
        alert(msg::fail_load_filesystem, stx::red);
}

int BootStartupConfig() {
    alert(msg::begin_startupcfg, stx::green);

    if (const Node* startupConfig = GetAbsolute("/boot/startupConfig.cmd")) {
        for (const std::string& line : split(startupConfig->value, '\n')) {
            CommandParams params = ParseCommandLine(line);
            Execute(params, true); // ignores poweroff and reboot
        }
    } else {
        alert(msg::startupcfg_not_exist, stx::yellow);
        Node* boot;
        if (!(boot = GetAbsolute("boot", FS::root)))
            boot = NewChild(FS::root, "boot", "dir");
        if (!GetAbsolute("boot/startupConfig.cmd", FS::root))
            NewChild(boot, "startupConfig", "cmd");
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
        if (size == 0)
            return std::filesystem::absolute(args[0]);

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

void BootFileDependencies() {
    using namespace std::filesystem;

    const path ramDir = SData::selfParentPath / "ram";
    SData::RAM::cpp = ramDir / "cppCompileable.cpp";
    SData::RAM::py = ramDir / "pythonExecutable.txt";
#ifdef _WIN32
    SData::RAM::exe = ramDir / "winExecutable.exe";
#else
    SData::RAM::exe = ramDir / "linuxExecutable";
#endif

    const path romDir = SData::selfParentPath / "rom";
    SData::ROM::fileSystem = romDir / "fileSystem.txt";

    SData::Export::selfDir = SData::selfParentPath / "export";

    create_directory(ramDir);
    create_directory(romDir);
    create_directory(SData::Export::selfDir);

    CreateMissingFile(SData::RAM::cpp);
    CreateMissingFile(SData::RAM::py);
    CreateMissingFile(SData::RAM::exe);

    CreateMissingFile(SData::ROM::fileSystem);
}

int Boot() {
    alert(msg::begin_boot, stx::green);

    BootFileDependencies();

    BootFileSystem();

    BootStartupConfig();

    SData::root = false;
    stx::ClearConsole();

    if (Login() == -1) return -1;

    stx::ClearConsole();
    std::cout << page::fetch << '\n';

    return Run();
}

int Post(char* args[]) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    SData::selfParentPath = GetExecutablePath(args).parent_path();

    while (true) {
        if (const int& returnCode = Boot(); returnCode != Reboot)
            return returnCode;

        SData::Reset();
        FS::Reset();
        stx::ClearConsole();
    }
}
