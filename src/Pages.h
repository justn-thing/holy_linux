#pragma once

namespace page {
    constexpr std::string_view fetch =
"Holy Linux\n\
Release: Release 1.0.0\n\
Developed and still in development by me, myself and I.\n";

    constexpr std::string_view help =
"sudo [cmd] [arg]\n\
-> run a command with admin permissions\n\
\n\
Navigation\n\
cd [arg]\n\
-> change directory (absolute or relative)\n\
dir / ls [arg]\n\
-> list directory contents (self, absolute or relative)\n\
\n\
Filesystem\n\
mkdir [name]\n\
-> create a directory (relative)\n\
rmdir [-r] [name]\n\
-> remove a directory (absolute or relative)\n\
mkfile [name.ext]\n\
-> create a file (relative)\n\
rmfile [name.ext]\n\
-> remove a file (absolute or relative)\n\
rename [path] [name]\n\
-> rename a directory or file (absolute or relative)\n\
lock [path]\n\
-> lock a file or directory (sudo only)\n\
unlock [path]\n\
-> unlock a file or directory (sudo only)\n\
\n\
Editing\n\
write / wr [name.ext]\n\
-> write a new file with Holy Vim\n\
edit [name.ext]\n\
-> edit an existing file with Holy Vim\n\
read [name.ext]\n\
-> print file contents\n\
\n\
Execution\n\
execute / exec [name.ext]\n\
-> run .cmd, .py, or .exe\n\
compile / comp [name.cpp]\n\
-> compile to .exe (requires g++)\n\
\n\
Mount\n\
mount / mnt [path] [name]\n\
-> mount an external .txt/.cmd/.py/.cpp into /root/mnt (sudo only)\n\
\n\
System\n\
help\n\
-> show this page\n\
clear\n\
-> clear the terminal\n\
fetch\n\
-> display the current release\n\
poweroff\n\
-> shutdown and save the filesystem\n";

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
