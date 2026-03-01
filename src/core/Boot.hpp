#pragma once

inline int Boot() {
    alert(msg::begin_boot, stx::green);

    alert(msg::begin_load_fs, stx::green);
    if (!loadFileSystem())
        alert(msg::fail_load_filesystem, stx::red);

    alert(msg::begin_startupcfg, stx::green);
    if (const Node* startupConfig = getAbsolute("/boot/startupConfig.cmd")) {
        for (const std::string line : split(startupConfig->value, '\n')) {
            if (CommandParams params = std::move(parseCommandLine(line));
                Execute(params, true) == 99)
                return 1;
        }
    } else
        alert(msg::fail_load_startupcfg, stx::red);

    SData::root = false;
    stx::clearConsole();

    if (Login() == -1)
        return -1;

    stx::clearConsole();
    std::cout << page::fetch;
    return Run();
}
