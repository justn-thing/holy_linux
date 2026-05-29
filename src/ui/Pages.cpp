#include "Pages.hpp"

#include <array>

#include "../Version.hpp"

namespace {
    template <std::size_t PrefixSize, std::size_t SuffixSize>
    constexpr auto MakeFetchText(const char (&prefix)[PrefixSize], const char (&suffix)[SuffixSize]) {
        std::array<char, PrefixSize + version::num.size() + SuffixSize - 2 + 1> text{};
        std::size_t index = 0;

        for (std::size_t i = 0; i < PrefixSize - 1; ++i) {
            text[index++] = prefix[i];
        }
        for (const char ch : version::num) {
            text[index++] = ch;
        }
        for (std::size_t i = 0; i < SuffixSize - 1; ++i) {
            text[index++] = suffix[i];
        }
        text[index] = '\0';

        return text;
    }

    constexpr auto fetchText = MakeFetchText(
"\033[33mHoly Linux\033[36m ","\n\
\033[90m========================\n\
\033[0mDeveloped by justn-thing\n");
}

namespace page {
    constexpr std::string_view fetch(fetchText.data(), fetchText.size() - 1);

    constexpr std::string_view help =
"\033[36mHoly Linux Command Help\033[0m\n\
\033[90mTip:\033[0m use \033[33msudo [command] [args]\033[0m for admin-only commands\n\
\033[90mFlags:\033[0m short flags use \033[33m-[flag]\033[0m, long flags use \033[33m--[flag]\033[0m\n\
\033[90mArgs:\033[0m [path] = virtual path, [file] = virtual file, [hostFile] = external file\n\
\033[90mOperators:\033[0m \033[33mcmd1 && cmd2\033[0m runs cmd2 after success, \033[33mcmd1 || cmd2\033[0m runs cmd2 after failure\n\
\033[90mRedirect:\033[0m \033[33mcmd > [file]\033[0m overwrites command output, \033[33mcmd >> [file]\033[0m appends command output\n\
\033[90mComments:\033[0m \033[33m// text\033[0m is ignored as a full-line .cmd comment\n\
\n\
\033[33mNavigation\033[0m\n\
\033[36mcd [path]\033[0m               change directory\n\
\033[36mdir / ls [path]\033[0m         list directory contents\n\
\033[36mpwd\033[0m                     print current path\n\
\n\
\033[33mFilesystem\033[0m\n\
\033[36mmkdir [path]\033[0m            create a directory\n\
\033[36mrmdir [-r] [path]\033[0m       remove a directory\n\
\033[36mmkfile / touch [file]\033[0m   create a file (default: .txt)\n\
\033[36mrmfile / rm [file]\033[0m      remove a file\n\
\033[36mrename [path] [newName]\033[0m rename directory or file\n\
\033[36mcopy / cp [srcPath] [destDir]\033[0m copy file or directory to directory\n\
\033[36mmove / mv [srcPath] [destDir]\033[0m move file or directory to directory\n\
\033[36mdu [path]\033[0m               show serialized file size (path optional)\n\
\033[36mtree [path]\033[0m             print recursive tree view (path optional)\n\
\033[36mfind [pattern] [path]\033[0m   recursively find files/directories (path optional)\n\
\033[36mlock [-r] [path]\033[0m        lock file or directory (sudo only, -r includes children)\n\
\033[36munlock [-r] [path]\033[0m      unlock file or directory (sudo only, -r includes children)\n\
\n\
\033[33mEditing\033[0m\n\
\033[36mwrite / wr [file]\033[0m       open Holy Vim and write file\n\
\033[36medit [file]\033[0m             edit existing file in Holy Vim\n\
\033[36mread / cat [file]\033[0m       print file contents\n\
\033[90msupported:\033[0m .txt .cmd .py .cpp\n\
\n\
\033[33mExecution\033[0m\n\
\033[36mexecute / exec [file]\033[0m      run .cmd, .py, or .exe\n\
\033[36mcompile / comp [file]\033[0m      compile .cpp into .exe (g++ required)\n\
\033[36m[command]\033[0m                  run package from /bin if found\n\
\033[90m.cmd:\033[0m lines beginning with \033[33m//\033[0m are comments; inline comments are not supported\n\
\n\
\033[33mImport / Export\033[0m\n\
\033[36mmount / mnt [hostFile]\033[0m  mount .txt/.cmd/.py/.cpp to /mnt (sudo only)\n\
\033[36mexport [file]\033[0m           write file to external export directory (sudo only)\n\
\n\
\033[33mSystem\033[0m\n\
\033[36mpassword / passwd [--root] [newPassword] [confirmPassword]\033[0m change password (--root: root password, sudo only)\n\
\033[36mhistory\033[0m                 show command history\n\
\033[36mecho [text]\033[0m             print text\n\
\033[36mwhoami\033[0m                  print current user\n\
\033[36mdate\033[0m                    print current date\n\
\033[36mtime\033[0m                    print current time\n\
\033[36mhelp\033[0m                    show this page\n\
\033[36mclear / cls\033[0m             clear terminal\n\
\033[36mfetch\033[0m                   display current release\n\
\033[36mpoweroff [-d]\033[0m           shutdown (-d discards changes)\n\
\033[36mreboot [-d]\033[0m             restart (-d discards changes)\n";

    constexpr std::string_view holyVim =
"Holy Vim 1.1\n\
\n\
Type ':cmds' for Commands\n";

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
:discard  | :d       exit without saving\n";
}
