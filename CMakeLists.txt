cmake_minimum_required(VERSION 3.20)
project(pirate-scrabble-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TARGET_NAME pirate-scrabble)

# Detect Emscripten and set flags BEFORE adding subdirectories
if (EMSCRIPTEN)
    message(STATUS "Building for Emscripten with pthreads")

    # IMPORTANT: Must be BEFORE adding Raylib
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -sUSE_PTHREADS=1")
elseif (UNIX AND NOT APPLE)
    # Enable AddressSanitizer for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "AddressSanitizer enabled")
        set(SANITIZER_FLAGS "-fsanitize=address -fsanitize=leak -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
    endif()
endif ()

add_subdirectory(external/raylib)
add_subdirectory(external/freetype)
add_subdirectory(external/harfbuzz)
add_subdirectory(external/concurrentqueue)
add_subdirectory(external/fmt)
add_subdirectory(external/frameflow)

if (NOT EMSCRIPTEN AND NOT WIN32)
    set(USE_TLS 1 CACHE INTERNAL "Force TLS")
    add_subdirectory(external/IXWebSocket)
endif ()

set(TARGET_SOURCES
        src/scrabble/main.cpp
        external/imgui/imgui.h
        external/imgui/imgui.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/misc/cpp/imgui_stdlib.h
        external/imgui/misc/cpp/imgui_stdlib.cpp
        external/imgui/misc/freetype/imgui_freetype.h
        external/imgui/misc/freetype/imgui_freetype.cpp
        external/rlImGui/rlImGui.cpp
        src/text/texthb.h
        src/text/texthb.cpp
        src/scrabble/context/socket_client.h
        src/scrabble/context/socket_client.cpp
        src/scrabble/context/main_menu.h
        src/scrabble/context/main_menu.cpp
        src/scrabble/context/login.cpp
        src/scrabble/context/login.h
        src/scrabble/context/multiplayer.cpp
        src/scrabble/context/multiplayer.h
        src/scrabble/context/types_inspector.h
        src/game_object/game_object.h
        src/game_object/game_object.cpp
        src/game_object/ui/control.cpp
        src/game_object/ui/control.h
        src/game_object/ui/layout_system.cpp
        src/game_object/ui/layout_system.h
        src/game_object/ui/drawing.h
        src/game_object/ui/drawing.cpp
        src/scrabble/sprites/tile.cpp
        src/scrabble/sprites/tile.h
        src/game_object/entity/entity.cpp
        src/game_object/entity/entity.h
        src/game_object/tween/tween.cpp
        src/game_object/tween/tween.h
        src/util/network/sockets/web_socket.h
        src/util/network/sockets/web_socket_desktop.h
        src/util/network/sockets/web_socket_web.h
        src/util/filesystem/filesystem.h
        src/util/queue.h
        src/util/math.h
        src/util/filesystem/filesystem.cpp
        src/game_object/entity/sprite.cpp
        src/game_object/entity/sprite.h
        src/util/logging/logging.h
        src/util/logging/logging.cpp
        src/util/scope_exit_callback.h
        src/util/serialization/serialization.h
        src/scrabble/actions/legal_actions.h
        src/scrabble/actions/legal_actions.cpp
)

if (EMSCRIPTEN)
    list(APPEND TARGET_SOURCES src/util/network/sockets/web_socket_web.cpp)
else ()
    list(APPEND TARGET_SOURCES src/util/network/sockets/web_socket_desktop.cpp)
endif ()

add_executable(${TARGET_NAME} ${TARGET_SOURCES})

target_include_directories(${TARGET_NAME}
        PRIVATE
        src
        external/imgui
        external/imgui/misc/cpp
        external/rlImGui
        external/cereal/include
        external/json
)

target_link_libraries(${TARGET_NAME}
        PRIVATE
        raylib
        frameflow::frameflow
        freetype
        harfbuzz
        concurrentqueue
        fmt::fmt
)

if (WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE Bcrypt)
endif()

if (NOT EMSCRIPTEN)
    target_link_libraries(${TARGET_NAME}
            PRIVATE
            ixwebsocket::ixwebsocket
    )
endif ()

target_compile_definitions(${TARGET_NAME} PRIVATE
        $<$<CONFIG:Debug>:BUILD_DEBUG>
        $<$<CONFIG:Release>:BUILD_RELEASE>
        $<$<CONFIG:RelWithDebInfo>:BUILD_RELWITHDEBINFO>
        $<$<CONFIG:MinSizeRel>:BUILD_MINSIZEREL>
)

if (EMSCRIPTEN)
    message(STATUS "Building for Emscripten")

    # Compile flags
    target_compile_options(${TARGET_NAME} PRIVATE
            -pthread
    )

    target_link_options(${TARGET_NAME} PRIVATE
            -pthread
            -sUSE_PTHREADS=1
            -sPTHREAD_POOL_SIZE=4
            -sFORCE_FILESYSTEM=1
            -sINITIAL_MEMORY=33554432  # 32MB instead of 16MB
            -sTOTAL_MEMORY=512MB
            #-sALLOW_MEMORY_GROWTH=1
            -sNO_DISABLE_EXCEPTION_CATCHING
            -sUSE_WEBGL2=1
            #-sFULL_ES3=1
            -lwebsocket.js
            --shell-file ${CMAKE_SOURCE_DIR}/shell.html
            --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets
    )

    set_target_properties(${TARGET_NAME} PROPERTIES
            SUFFIX ".html"
    )
endif ()

if (NOT EMSCRIPTEN)
    # Create a custom target that always runs
    add_custom_target(copy_assets_target ALL
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${TARGET_NAME}>/assets
            COMMENT "Copying assets..."
            VERBATIM
    )

    # Make sure it runs before your target links
    add_dependencies(${TARGET_NAME} copy_assets_target)
endif ()

# Build timestamp
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S" UTC)

target_compile_definitions(${TARGET_NAME} PRIVATE BUILD_TIMESTAMP_UTC="${BUILD_TIMESTAMP}")

find_package(Git REQUIRED)

execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

target_compile_definitions(${TARGET_NAME} PRIVATE
        GIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"
)

find_package(Doxygen REQUIRED)

set(DOXYFILE_IN  ${CMAKE_SOURCE_DIR}/Doxyfile)
set(DOXYFILE_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(GLOB_RECURSE DOXY_INPUTS
        ${CMAKE_SOURCE_DIR}/src/*.h
        ${CMAKE_SOURCE_DIR}/include/*.h)

add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        DEPENDS ${DOXY_INPUTS}
)

