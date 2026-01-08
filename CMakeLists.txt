cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/raylib)
add_subdirectory(external/frameflow)
add_subdirectory(external/freetype)
add_subdirectory(external/harfbuzz)

add_executable(game
    src/main.cpp
)

target_include_directories(game
    PRIVATE
        src
)

target_link_libraries(game
    PRIVATE
        raylib
        frameflow::frameflow
        freetype
        harfbuzz
)
