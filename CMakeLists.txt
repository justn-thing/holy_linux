cmake_minimum_required(VERSION 3.20)
project(holy_linux)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(holy_linux src/main.cpp)
set_target_properties(holy_linux PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
)

target_link_options(holy_linux PRIVATE
        -static-libstdc++
        -static-libgcc
)

if(MINGW)
    target_link_options(holy_linux PRIVATE
            -static
    )
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    get_filename_component(MINGW_ROOT "${MINGW_BIN_DIR}" DIRECTORY)
    find_library(WINPTHREAD_STATIC
        NAMES libwinpthread.a winpthread
        PATHS "${MINGW_ROOT}/x86_64-w64-mingw32/lib"
    )
    if(WINPTHREAD_STATIC)
        target_link_libraries(holy_linux PRIVATE
            -Wl,-Bstatic
            "${WINPTHREAD_STATIC}"
            -Wl,-Bdynamic
        )
    else()
        target_link_libraries(holy_linux PRIVATE
            -Wl,-Bstatic
            -l:libwinpthread.a
            -Wl,-Bdynamic
        )
    endif()
else()
    find_package(Threads REQUIRED)
    target_link_libraries(holy_linux PRIVATE Threads::Threads)
endif()
