# holy_linux

Small C++23 "toy OS" shell that simulates a filesystem in memory and persists it to disk.

## What It Does

- Boots, runs a scripted startup config, then drops into an interactive prompt.
- Manages a virtual filesystem (directories and files) under `/root`.
- Provides a mini editor ("Holy Vim") and simple command execution.
- Persists state to `rom/` and temporary execution buffers to `ram/`.

## Build

Requires CMake 4.0+ and a C++23-capable compiler.

```sh
cmake -S . -B cmake-build
cmake --build cmake-build
```

The binary is written to `build/holy_linux.exe` (per `CMakeLists.txt`).

## Run

Run from the project root so the program can find `rom/` and `ram/`:

```sh
build/holy_linux.exe
```

## Project Layout

- `src/` C++ sources
- `rom/` persistent storage
  - `rom/startup.txt` boot-time command script
  - `rom/fileSystem.txt` serialized filesystem
- `ram/` temp files used by `execute` and `compile`
- `build/` output directory

## Commands (User-Facing)

This is the in-app help text, summarized:

- `sudo [cmd] [arg]` run a command with admin permissions
- `cd [arg]` change directory (absolute or relative)
- `dir` / `ls [arg]` list directory contents
- `mkdir [arg]`, `rmdir [-r] [arg]` create/remove directory
- `mkfile` / `touch [name.ext]` create a file (defaults to `.txt`)
- `rmfile` / `rm [name.ext]` remove a file
- `rename [path] [name]` rename directory or file
- `write` / `wr [name.ext]` open Holy Vim editor
- `edit [name.ext]` edit while keeping existing content
- `read` / `cat [name.ext]` print file contents
- `execute` / `exec [name.ext]` run `.cmd`, `.py`, or `.exe`
- `compile` / `comp [name.cpp]` compile to `.exe`
- `mount [path] [name]` mount external file to `/root/mnt` (sudo only)
- `help`, `clear`, `cls`, `fetch`, `pwd`, `poweroff [-d]` (use `-d` to discard changes)

## Notes

- Paths are rooted at `/root`; `cd ..` moves up.
- `execute` uses local tooling (`py`, `g++`) if present.
- `compile` expects `g++` on PATH and writes an `.exe` into the virtual FS.
