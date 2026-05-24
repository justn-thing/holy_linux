# Holy Linux

Small C++23 simulated OS shell that keeps a virtual filesystem in memory and persists it to `rom/fileSystem.txt`.

## Features

- Boot flow with startup script execution (`/boot/startupConfig.cmd` inside the virtual filesystem).
- Multi-user login (each directory inside `/home` counts as a user) with per-user passwords stored in `/etc/login.txt` (virtual filesystem file).
- Root escalation via `sudo` (prompts for root password when needed).
- In-shell filesystem operations (`cd`, `mkdir`, `touch`, `rm`, `rename`, `copy`, `move`, locking).
- Built-in editor (`Holy Vim`) for `.txt`, `.cmd`, `.py`, and `.cpp` files.
- Script/program execution (`.cmd`, `.py`, `.exe`) and C++ compilation to virtual `.exe`.

## Build

Requires CMake and a C++23-capable compiler.

```sh
cmake -S . -B cmake-build
cmake --build cmake-build
```

Per `CMakeLists.txt`, runtime output is placed in `build/`.

## Requirements

- C++23 toolchain (project is built for MinGW/g++ workflow)
- CMake
- `g++` on PATH (used by in-app `compile` for `.cpp`)
- `py` (Windows) or `python3` (Linux) on PATH (used by in-app `execute` for `.py`)

## Run

Run the built executable:

```sh
build/holy_linux.exe
```

Runtime directories are created next to the executable.

## Runtime Directories

- `rom/` persistent virtual filesystem image (`fileSystem.txt`)
- `ram/` temp files used by compiled/executable payloads
- `export/` exported files

## Quick Demo

Example first-run flow:

```text
mkdir test
cd test
touch hello.txt
write hello.txt
read hello.txt
password newpass newpass
pwd
poweroff
```

## Commands

- `sudo [command] [args]` run a command with root privileges
- Short flags use `-[flag]`, long flags use `--[flag]`; unknown flags are rejected.
- Placeholder guide: `[path]` is a virtual filesystem path, `[file]` is a virtual file path, and `[hostFile]` is an external file path.

Navigation
- `cd [path]` change directory
- `dir` / `ls [path]` list directory contents
- `pwd` print working directory

Filesystem
- `mkdir [path]` create directory
- `rmdir [-r] [path]` remove directory
- `mkfile` / `touch [file]` create file (defaults to `.txt`)
- `rmfile` / `rm [file]` remove file
- `rename [path] [newName]` rename file or directory
- `copy` / `cp [srcPath] [destDir]` copy file or directory to directory
- `move` / `mv [srcPath] [destDir]` move file or directory to directory
- `du [path]` show serialized file size (`path` optional, defaults to current directory)
- `tree [path]` print a recursive tree view (`path` optional, defaults to current directory)
- `find [pattern] [path]` recursively find matching files/directories (`path` optional, defaults to current directory)
- `lock [-r] [path]` lock node (sudo only, `-r` includes children)
- `unlock [-r] [path]` unlock node (sudo only, `-r` includes children)

Editing
- `write` / `wr [file]` open Holy Vim for supported text types
- `edit [file]` edit existing file in Holy Vim
- `read` / `cat [file]` print file contents

Execution
- `execute` / `exec [file]` run `.cmd`, `.py`, or `.exe`
- `compile` / `comp [file]` compile `.cpp` to `.exe` (requires `g++`)
- `[command]` run matching `.exe`, `.cmd`, or `.py` package from `/bin`

Import / Export
- `mount` / `mnt [hostFile]` import external `.txt`/`.cmd`/`.py`/`.cpp` into `/mnt` using its external filename (sudo only)
- `export [file]` write a virtual file to the external `export` directory (sudo only)

System
- `password` / `passwd [--root] [newPassword] [confirmPassword]` change password (`--root` changes root password, sudo only)
- `history` show command history
- `echo [text]` print text
- `whoami` print current user
- `date` print current date
- `time` print current time
- `help` show help page
- `clear` / `cls` clear terminal
- `fetch` display release banner
- `poweroff [-d]` shutdown (`-d` discards changes)
- `reboot [-d]` restart (`-d` discards changes)
