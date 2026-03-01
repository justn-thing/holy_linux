#pragma once

namespace page {
    constexpr std::string_view fetch =
"\033[33mHoly Linux\033[36m 2.0\n\
\033[90m==============================\n\
\033[0mDeveloped by me, myself and I.\n\n";

    constexpr std::string_view help =
"sudo [cmd] [arg]  run a command with admin permissions\n\
\n\
\033[90mNavigation\033[0m\n\
- cd [arg]         change directory\n\
- dir / ls [arg]   list directory contents\n\
- pwd              print the current path\n\
\n\
\033[90mFilesystem\033[0m\n\
- mkdir [name]           create a directory\n\
- rmdir [-r] [name]      remove a directory\n\
- mkfile / touch [name]  create a file (defaults to .txt)\n\
- rmfile / rm [name.ext] remove a file\n\
- rename [path] [name]   rename a directory or file\n\
- copy / cp [src] [dest] copy a file to a directory (dest optional)\n\
- move / mv [src] [dest] move a file to a directory (dest optional)\n\
- lock [path]            lock a file or directory (sudo only)\n\
- unlock [path]          unlock a file or directory (sudo only)\n\
\n\
\033[90mEditing\033[0m\n\
- write / wr [name.ext]  write a file with Holy Vim (.txt/.cmd/.py/.cpp)\n\
- edit [name.ext]        edit a file with Holy Vim (.txt/.cmd/.py/.cpp)\n\
- read / cat [name.ext]  print file contents\n\
\n\
\033[90mExecution\033[0m\n\
- execute / exec [name.ext] run .cmd, .py, or .exe\n\
- compile / comp [name.cpp] compile a .cpp into .exe (requires g++ on PATH)\n\
\n\
\033[90mMount\033[0m\n\
- mount / mnt [path] [name.ext] mount an external .txt/.cmd/.py/.cpp into /root/mnt (sudo only)\n\
\n\
\033[90mSystem\033[0m\n\
- password / passwd [new] [confirm]  change current user's password\n\
- help               show this page\n\
- clear / cls        clear the terminal\n\
- fetch              display the current release\n\
- poweroff [-d]      shutdown (use -d to discard changes)\n\n";

    constexpr std::string_view holyVim =
"Holy Vim 1.1\n\
\n\
Type ':cmds' for Commands\n\n";

    constexpr std::string_view holyVimHelp =
"Holy Vim Commands\n\
\n\
:goto #   | :gt #    jump to line #\n\
:insert # | :ins #   insert line before #\n\
:delete # | :del #   delete line #\n\
\n\
:commands | :cmds    view this page\n\
:reload   | :re      view the entire file in order\n\
\n\
:save     | :s       save and exit\n\
:discard  | :d       exit without saving\n\n";
}
