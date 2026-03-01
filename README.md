# Holy Linux

Small C++23 simulated OS shell that keeps a virtual filesystem in memory and persists it to `rom/fileSystem.txt`.

## Features

- Boot flow with startup script execution (`/boot/startupConfig.cmd` inside the virtual filesystem).
- Multi-user login with per-user passwords stored in `/etc/login.txt` (virtual filesystem file).
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
- `g++` on `PATH` (required by in-app `compile` / `comp`)
- `py` (Windows) or `python3` (Linux) on `PATH` (used by in-app `execute` for `.py`)
- Run from repository root so `rom/` and `ram/` paths resolve

## Run

Run from project root so relative `rom/` and `ram/` paths resolve correctly:

```sh
build/holy_linux.exe
```

## Runtime Directories

- `rom/` persistent virtual filesystem image (`fileSystem.txt`)
- `ram/` temp files used by `execute`/`compile`
- `src/` source code

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

- `sudo [cmd] [arg]` run a command with root privileges

Navigation
- `cd [arg]` change directory
- `dir` / `ls [arg]` list directory contents
- `pwd` print current path

Filesystem
- `mkdir [name]` create directory
- `rmdir [-r] [name]` remove directory
- `mkfile` / `touch [name]` create file (defaults to `.txt`)
- `rmfile` / `rm [name.ext]` remove file
- `rename [path] [name]` rename file or directory
- `copy` / `cp [src] [dest]` copy file to directory (`dest` optional, defaults to current directory)
- `move` / `mv [src] [dest]` move file to directory (`dest` optional, defaults to current directory)
- `lock [path]` lock node (sudo only)
- `unlock [path]` unlock node (sudo only)

Editing
- `write` / `wr [name.ext]` open Holy Vim for supported text types
- `edit [name.ext]` edit existing file in Holy Vim
- `read` / `cat [name.ext]` print file contents

Execution
- `execute` / `exec [name.ext]` run `.cmd`, `.py`, or `.exe`
- `compile` / `comp [name.cpp]` compile to `.exe` (requires `g++`)

Mount
- `mount` / `mnt [path] [name.ext]` import external `.txt`/`.cmd`/`.py`/`.cpp` into `/root/mnt` (sudo only)

System
- `password` / `passwd [new] [confirm]` change current user password
- `help` show help page
- `clear` / `cls` clear terminal
- `fetch` display release banner
- `poweroff [-d]` shutdown (`-d` discards changes)
