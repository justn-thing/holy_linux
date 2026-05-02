cmake_minimum_required(VERSION 3.20)
project(holy_linux)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(holy_linux
        src/main.cpp
        src/apps/HolyVim.cpp
        src/cmd/CommandParser.cpp
        src/cmd/Execution.cpp
        src/core/Boot.cpp
        src/core/Run.cpp
        src/fs/FileSaving.cpp
        src/fs/FileTree.cpp
        src/fs/NodeStruct.cpp
        src/session/Login.cpp
        src/session/SessionData.cpp
        src/ui/Messages.cpp
        src/ui/Pages.cpp
        src/ui/Syntax.cpp
        src/util/Misc.cpp
)

set_target_properties(holy_linux PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)

target_link_options(holy_linux PRIVATE
        -static
        -static-libstdc++
        -static-libgcc
)
