#include "Pages.hpp"

namespace page {
    const std::string_view fetch =
"\033[33mHoly Linux\033[36m 2.2\n\
\033[90m==============================\n\
\033[0mDeveloped by me, myself and I.\n\n";

    const std::string_view help =
"\033[36mHoly Linux Command Help\033[0m\n\
\033[90mTip:\033[0m use \033[33msudo [cmd]\033[0m for admin-only commands\n\
\n\
\033[33mNavigation\033[0m\n\
\033[36mcd [path]\033[0m               change directory\n\
\033[36mdir / ls [path]\033[0m         list directory contents\n\
\033[36mpwd\033[0m                     print current path\n\
\n\
\033[33mFilesystem\033[0m\n\
\033[36mmkdir [name]\033[0m            create a directory\n\
\033[36mrmdir [-r] [name]\033[0m       remove a directory\n\
\033[36mmkfile / touch [name]\033[0m   create a file (default: .txt)\n\
\033[36mrmfile / rm [name]\033[0m      remove a file\n\
\033[36mrename [path] [name]\033[0m    rename directory or file\n\
\033[36mcopy / cp [src] [dest]\033[0m  copy file to directory (dest optional)\n\
\033[36mmove / mv [src] [dest]\033[0m  move file to directory (dest optional)\n\
\033[36mlock [path]\033[0m             lock file or directory (sudo only)\n\
\033[36munlock [path]\033[0m           unlock file or directory (sudo only)\n\
\n\
\033[33mEditing\033[0m\n\
\033[36mwrite / wr [name]\033[0m   open Holy Vim and write file\n\
\033[36medit [name]\033[0m         edit existing file in Holy Vim\n\
\033[36mread / cat [name]\033[0m   print file contents\n\
\033[90msupported:\033[0m .txt .cmd .py .cpp\n\
\n\
\033[33mExecution\033[0m\n\
\033[36mexecute / exec [name]\033[0m      run .cmd, .py, or .exe\n\
\033[36mcompile / comp [name.cpp]\033[0m  compile .cpp into .exe (g++ required)\n\
\033[36m[name]\033[0m                     run package from /bin if found\n\
\n\
\033[33mMount\033[0m\n\
\033[36mmount / mnt [hostPath] [name]\033[0m  mount .txt/.cmd/.py/.cpp to /root/mnt (sudo only)\n\
\n\
\033[33mSystem\033[0m\n\
\033[36mpassword / passwd [new] [confirm]\033[0m   change current user's password\n\
\033[36mhistory\033[0m                 show command history\n\
\033[36mecho [text]\033[0m             print text\n\
\033[36mwhoami\033[0m                  print current user\n\
\033[36mdate\033[0m                    print current date\n\
\033[36mtime\033[0m                    print current time\n\
\033[36mhelp\033[0m                    show this page\n\
\033[36mclear / cls\033[0m             clear terminal\n\
\033[36mfetch\033[0m                   display current release\n\
\033[36mpoweroff [-d]\033[0m           shutdown (-d discards changes)\n\n";

    const std::string_view holyVim =
"Holy Vim 1.1\n\
\n\
Type ':cmds' for Commands\n\n";

    const std::string_view holyVimHelp =
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
