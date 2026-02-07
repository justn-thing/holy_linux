#pragma once

namespace page {
    constexpr std::string_view fetch =
"Holy Linux\n\
Release: Release 1.0.0\n\
Developed and still in development by me, myself and I.\n";

    constexpr std::string_view help =
"sudo [cmd] [arg]  run a command with admin permissions\n\
\n\
Navigation\n\
- cd [arg]         change directory (absolute or relative)\n\
- dir / ls [arg]   list directory contents (self, absolute or relative)\n\
- pwd              print the current path\n\
\n\
Filesystem\n\
- mkdir [name]           create a directory (relative)\n\
- rmdir [-r] [name]      remove a directory (absolute or relative)\n\
- mkfile / touch [name]  create a file (relative, defaults to .txt)\n\
- rmfile / rm [name.ext] remove a file (absolute or relative)\n\
- rename [path] [name]   rename a directory or file (absolute or relative)\n\
- lock [path]            lock a file or directory (sudo only)\n\
- unlock [path]          unlock a file or directory (sudo only)\n\
\n\
Editing\n\
- write / wr [name.ext]  write a new file with Holy Vim\n\
- edit [name.ext]        edit an existing file with Holy Vim\n\
- read / cat [name.ext]  print file contents\n\
\n\
Execution\n\
- execute / exec [name]  run .cmd, .py, or .exe\n\
- compile / comp [name]  compile a .cpp into .exe (requires g++)\n\
\n\
Mount\n\
- mount / mnt [path] [name]  mount an external .txt/.cmd/.py/.cpp into /root/mnt (sudo only)\n\
\n\
System\n\
- help              show this page\n\
- clear / cls        clear the terminal\n\
- fetch              display the current release\n\
- poweroff [-d]      shutdown (use -d to discard changes)\n";

    constexpr std::string_view holyVim =
"Holy Vim | Release 1.1\n\
\n\
Type text to add or replace a line.\n\
\n\
Commands\n\
:goto #    jump to line\n\
:ins #     insert line before #\n\
:del #     delete line #\n\
:save      save and exit\n\
:discard   exit without saving\n";

}
