#include "Boot.hpp"

#include <iostream>
#include <string>

#include "../cmd/CommandParser.hpp"
#include "../cmd/Execution.hpp"
#include "../fs/FileSaving.hpp"
#include "../fs/FileTree.hpp"
#include "../session/Login.hpp"
#include "../session/SessionData.hpp"
#include "../ui/Messages.hpp"
#include "../ui/Pages.hpp"
#include "../ui/Syntax.hpp"
#include "../util/Misc.hpp"
#include "Run.hpp"

void BootFileSystem() {
    alert(msg::begin_load_fs, stx::green);

    if (!LoadFileSystem())
        alert(msg::fail_load_filesystem, stx::red);
}

int BootStartupConfig() {
    alert(msg::begin_startupcfg, stx::green);

    if (const Node* startupConfig = GetAbsolute("/boot/startupConfig.cmd")) {
        for (const std::string& line : split(startupConfig->value, '\n')) {
            if (CommandParams params = parseCommandLine(line);
                Execute(params, true) == 99)
                return 99;
        }
    } else {
        alert(msg::fail_load_startupcfg, stx::red);
    }

    return 0;
}

int Boot() {
    alert(msg::begin_boot, stx::green);

    BootFileSystem();

    if (BootStartupConfig() == 99)
        return 1;

    SData::root = false;
    stx::ClearConsole();

    if (Login() == -1)
        return -1;

    stx::ClearConsole();
    std::cout << page::fetch;

    return Run();
}
