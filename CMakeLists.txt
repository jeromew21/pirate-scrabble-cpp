cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect Emscripten and set flags BEFORE adding subdirectories
if(EMSCRIPTEN)
    message(STATUS "Building for Emscripten with pthreads")

    # IMPORTANT: Must be BEFORE adding Raylib
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -sUSE_PTHREADS=1")
endif()

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

set(SOURCES
        src/main.cpp
        external/imgui/imgui.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/misc/cpp/imgui_stdlib.cpp
        external/rlImGui/rlImGui.cpp
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
        src/util/network/sockets/web_socket.h
        src/util/network/sockets/web_socket_desktop.h
        src/util/network/sockets/web_socket_web.h
        src/util/filesystem.h
        src/util/queue.h
        src/util/math.h
        src/util/filesystem.cpp
        src/game_object/entity/sprite.cpp
        src/game_object/entity/sprite.h
        src/serialization/types_inspector.h
        src/util/logging/logging.h
)

if (EMSCRIPTEN)
    list(APPEND SOURCES src/util/network/sockets/web_socket_web.cpp)
else ()
    list(APPEND SOURCES src/util/network/sockets/web_socket_desktop.cpp)
endif ()

add_executable(game ${SOURCES})

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
)

if (NOT EMSCRIPTEN)
    target_link_libraries(game
            PRIVATE
        ixwebsocket::ixwebsocket
    )
endif()

# Detect Emscripten
if(EMSCRIPTEN)
    message(STATUS "Building for Emscripten")

    # Compile flags
    target_compile_options(${PROJECT_NAME} PRIVATE
            -pthread
    )

    target_link_options(${PROJECT_NAME} PRIVATE
            -pthread
            -sUSE_PTHREADS=1
            -sPTHREAD_POOL_SIZE=4
            -sFORCE_FILESYSTEM=1
            -sINITIAL_MEMORY=33554432  # 32MB instead of 16MB
            -sTOTAL_MEMORY=512MB
            #-sALLOW_MEMORY_GROWTH=1
            -sNO_DISABLE_EXCEPTION_CATCHING
            -lwebsocket.js  # This is what you were missing!
            --shell-file ${CMAKE_SOURCE_DIR}/shell.html
            --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets
    )

    set_target_properties(game PROPERTIES
            SUFFIX ".html"
    )
endif()