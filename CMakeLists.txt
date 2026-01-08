cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/raylib)
add_subdirectory(external/frameflow)
add_subdirectory(external/freetype)
add_subdirectory(external/harfbuzz)

set(USE_TLS 1 CACHE INTERNAL "Force TLS")
add_subdirectory(external/IXWebSocket)

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
        ixwebsocket::ixwebsocket
)
