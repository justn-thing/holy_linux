cmake_minimum_required(VERSION 4.0)
project(holy_linux)

set(CMAKE_CXX_STANDARD 23)

add_executable(holy_linux src/main.cpp)
set_target_properties(holy_linux PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)