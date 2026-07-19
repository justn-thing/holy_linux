cmake_minimum_required(VERSION 3.20)
project(holy_linux)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(holy_linux ${PROJECT_SOURCES})

set_target_properties(holy_linux PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)

target_link_options(holy_linux PRIVATE
        -static-libstdc++
        -static-libgcc
)
