cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/raylib)
add_subdirectory(external/frameflow)
add_subdirectory(external/freetype)
add_subdirectory(external/harfbuzz)
add_subdirectory(external/concurrentqueue)
add_subdirectory(external/fmt)

set(USE_TLS 1 CACHE INTERNAL "Force TLS")
add_subdirectory(external/IXWebSocket)

add_executable(game
        src/main.cpp
        external/imgui/imgui.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/misc/cpp/imgui_stdlib.cpp
        external/rlImGui/rlImGui.cpp
        src/util/util.h
        src/util/util.cpp
        src/text/texthb.h
        src/text/texthb.cpp
        src/context/sockets.h
        src/context/sockets.cpp
        src/context/main_menu.h
        src/context/main_menu.cpp
        src/context/login.cpp
        src/context/login.h
        src/context/multiplayer.cpp
        src/context/multiplayer.h
        src/game_object/game_object.h
        src/game_object/game_object.cpp
        src/game_object/ui/control.cpp
        src/game_object/ui/control.h
        src/game_object/ui/layout_system.cpp
        src/game_object/ui/layout_system.h
        src/game_object/ui/drawing.h
        src/game_object/ui/drawing.cpp
        src/scrabble/tile.cpp
        src/scrabble/tile.h
        src/game_object/entity/entity.cpp
        src/game_object/entity/entity.h
)

target_include_directories(game
        PRIVATE
        src
        external/imgui
        external/imgui/misc/cpp
        external/rlImGui
        external/cereal/include
        external/json
)

target_link_libraries(game
        PRIVATE
        raylib
        frameflow::frameflow
        freetype
        harfbuzz
        concurrentqueue
        fmt::fmt
        ixwebsocket::ixwebsocket
)
