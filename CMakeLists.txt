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

if (NOT EMSCRIPTEN)
    set(USE_TLS 1 CACHE INTERNAL "Force TLS")
    add_subdirectory(external/IXWebSocket)
endif()

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
        src/game_object/tween/tween.cpp
        src/game_object/tween/tween.h
        src/network/sockets/web_socket.h
        src/network/sockets/web_socket_desktop.h
        src/network/sockets/web_socket_web.h
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

# Detect Emscripten
if(EMSCRIPTEN)
    message(STATUS "Building for Emscripten")

    # Enable pthreads
    set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "-s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4"
            COMPILE_FLAGS "-pthread"
    )

    # Enable WebSockets
    set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s USE_WEBSOCKET=1"
    )

    # Enable filesystem (optional, if you want read/write access)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s FORCE_FILESYSTEM=1"
    )

    # Optional: set main loop style for async rendering
    set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s EXIT_RUNTIME=1"
    )
endif()
