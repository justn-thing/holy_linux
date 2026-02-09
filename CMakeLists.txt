cmake_minimum_required(VERSION 3.20)
project(holy_linux)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(holy_linux src/main.cpp)
set_target_properties(holy_linux PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
        BUILD_RPATH "$ORIGIN/lib"
        INSTALL_RPATH "$ORIGIN/lib"
)

target_link_options(holy_linux PRIVATE
        -static-libstdc++
        -static-libgcc
)
