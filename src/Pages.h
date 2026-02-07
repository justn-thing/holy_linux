#pragma once

namespace page {
    constexpr std::string_view fetch =
"Holy Linux\n\
Release: Release 1.0.0\n\
Developed and still in development by me, myself and I.\n";

    constexpr std::string_view help =
"sudo [cmd] [arg]\n\
-> grants administrator permissions\n\
cd [arg]\n\
-> enters a directory (access: absolute or relative)\n\
dir / ls [arg]\n\
-> shows contents of a directory (access: self, absolute or relative)\n\
mkdir [arg]\n\
-> creates a directory (access: relative)\n\
rmdir [-r]? [arg]\n\
-> removes a directory (access: absolute, relative)\n\
mkfile [arg.ext]\n\
-> creates a file (access: relative)\n\
rmfile [arg.ext]\n\
-> creates a file (access: absolute, relative)\n\
rename [arg.ext] [name]\n\
-> renames a file (access: absolute, relative)\n\
write / wr [arg.ext]\n\
-> writes inside a file using Holy Vim (access: relative)\n\
edit [arg.ext]\n\
-> writes inside a file using Holy Vim while preserving previous content (access: relative)\n\
read [arg.ext]\n\
-> reads out the contents of a file (access: relative)\n\
execute / exec [arg.ext]\n\
-> executes the contents of a file (access: relative)\n\
compile / comp [arg.ext]\n\
-> compiles the contents of a file into .exe (access: relative)\n\
mount / mnt [arg] [name]\n\
-> mounts an external file into /root/mnt\n\
-> requires administrator permissions\n\
help\n\
-> shows this page\n\
clear\n\
-> clears the terminal\n\
fetch\n\
-> displays the current release\n\
poweroff\n\
-> shuts down the system and saves files\n";

    constexpr std::string_view holyVim =
"Holy Vim | Release 1.1\n\
\n\
Go to line with: ':goto #'\n\
Insert line with: ':ins #'\n\
Delete line with: ':del #'\n\
Save & Exit with: ':save'\n\
Discard & Exit with: ':discard'\n";

}