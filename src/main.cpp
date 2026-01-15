#include <string>
#include <chrono>

#include "raylib.h"
#include "rlImGui.h"

#include "imgui.h"
#include "misc/freetype/imgui_freetype.h"

#include "frameflow/layout.hpp"

#include "text/freetype_library.h"
#include "text/texthb.h"
#include "serialization/types.h"
#include "context/main_menu.h"
#include "context/multiplayer.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "game_object/tween/tween.h"
#include "util/filesystem/filesystem.h"
#include "util/logging/logging.h"
#include "util/scope_exit_callback.h"
#include "scrabble/tile.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static std::function<void()> main_loop_function;

extern "C" void loop_wrapper() {
    main_loop_function();
}

using namespace frameflow;

struct Performance {
    double update_time = 0.0;
    int update_count = 0;
    double draw_time = 0.0;
    int draw_count = 0;
    double update_avg = 0;
    double draw_avg = 0;
    std::chrono::high_resolution_clock::time_point last_print;

    Performance() : last_print(std::chrono::high_resolution_clock::now()) {
    }
};

struct Profiler {
    const char *name;
    double &accumulator;
    int &count;
    std::chrono::high_resolution_clock::time_point start;

    Profiler(const char *n, double &acc, int &c) : name(n),
                                                   accumulator(acc),
                                                   count(c),
                                                   start(std::chrono::high_resolution_clock::now()) {
    }

    ~Profiler() {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto ms = std::chrono::duration<double, std::milli>(end - start).count();
        accumulator += ms;
        count++;
    }
};

void InitCrossPlatformWindow(const int logical_width, const int logical_height, const char *title) {
    SetTraceLogLevel(LOG_NONE);
    constexpr unsigned int flags = FLAG_WINDOW_RESIZABLE
                                   | FLAG_MSAA_4X_HINT
                                   | FLAG_VSYNC_HINT;
    SetConfigFlags(flags);

    // Initialize with logical dimensions - raylib handles DPI internally
    InitWindow(logical_width, logical_height, title);
    SetExitKey(KEY_NULL);
#ifdef __EMSCRIPTEN__
    // Get the actual canvas size set by JavaScript
    // Somehow this isn't working quite properly, you have to manual resize before it works
    int canvas_width = EM_ASM_INT({return Module.canvas.width;});
    int canvas_height = EM_ASM_INT({return Module.canvas.height;});
#endif
    SetWindowSize(logical_width, logical_height);
}

float GetLogicalRatio() {
    return static_cast<float>(GetScreenWidth()) / static_cast<float>(GetRenderWidth());
}

static PersistentData load_persistent_data(const std::string &persistent_data_path) {
    if (std::string contents; read_file(persistent_data_path, contents)) {
        try {
            return deserialize_or_throw<PersistentData>(contents);
        } catch (const std::exception &e) {
            Logger::instance().info("Falling back to default persistent data: {}", e.what());
            return PersistentData{};
        }
    }
    Logger::instance().info("Falling back to default persistent data");
    return PersistentData{};
}

static bool write_persistent_data_to_disk(const std::string &persistent_data_path, const PersistentData &data) {
    return write_file(persistent_data_path, serialize(data));
}

float Tile::dim = 81;

int main() {
    // -------------------------
    // Initialize logging
    // -------------------------
    Logger::Initialize("pirate_scrabble.log");

    // -------------------------
    // Initialize persistent data
    // -------------------------
    const std::string persistent_data_path{"./pirate_scrabble_data.json"};
    PersistentData persistent_data = load_persistent_data("./pirate_scrabble_data.json");
    ScopeExitCallback persistent_data_exit([&] {
        if (!write_persistent_data_to_disk(persistent_data_path, persistent_data)) {
            Logger::instance().info("Failed to write persistent data to disk");
        }
    });

    // -------------------------
    // Initialize raylib
    // -------------------------
    InitCrossPlatformWindow(persistent_data.window_width,
                            persistent_data.window_height,
                            "Pirate Scrabble");
    // maybe draw an initial image?

    // -------------------------
    // Initialize FreeType
    // -------------------------
    auto *ft = ft_init();

    // -------------------------
    // Initialize tile textures
    // -------------------------
    Tile::InitializeTextures(ft);

    // -------------------------
    // Initialize ImGui
    // -------------------------
    rlImGuiSetup(true);
    const ImGuiIO &io = ImGui::GetIO();
    io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
    const auto ibm_plex_mono = FS_ROOT / "assets" / "IBM_Plex_Mono" / "IBMPlexMono-Light.ttf";
    ImFont *imgui_font = io.Fonts->AddFontFromFileTTF(ibm_plex_mono.c_str(),
                                                      32.0f * GetLogicalRatio());

    const auto arial = FS_ROOT / "assets" / "arial.ttf";
    const auto face = ft_load_font(ft, arial);

    HBFont font(face, 48); // pixel size 48

    // -------------------------
    // Initialize context. We have to be careful that this doesn't initialize any sprites.
    // We want to read previous window size here though. Maybe move out of menu context?
    // -------------------------
    GameObject root{};
    MainMenuContext menu_context{};
    root.AddChild(&menu_context);

    // -------------------------
    // Initialize performance tracker
    // -------------------------
    Performance perf{};

    // -------------------------
    // Cleanup
    // -------------------------
    ScopeExitCallback cleanup_exit([] {
        // todo: shutdown harfbuzz/freetype
        // todo: recursively delete root
        Logger::instance().info("Shutting down");
        rlImGuiShutdown();
        CloseWindow();
    });

    main_loop_function = [&] {
        if (IsWindowResized()) {
#ifdef __EMSCRIPTEN__
            int canvas_width = EM_ASM_INT({return Module.canvas.width;});
            int canvas_height = EM_ASM_INT({return Module.canvas.height;});
            SetWindowSize(canvas_width, canvas_height);
            Logger::instance().info("Resized to: {}, {}", canvas_width, canvas_height);
#else
            //SetWindowSize(, 800);
#endif
            persistent_data.window_width = GetScreenWidth();
            persistent_data.window_height = GetScreenHeight();
            //fix this!
        }

        // Update
        {
            Profiler p("UpdateRec", perf.update_time, perf.update_count);
            const float dt = GetFrameTime();
            root.UpdateRec(dt);
            TweenManager::instance().Update(dt);
        }

        // Draw
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);
            rlImGuiBegin();
            ImGui::PushFont(imgui_font);
            {
                Profiler p("DrawRec", perf.draw_time, perf.draw_count);
                root.DrawRec();
            }
            const auto now = std::chrono::high_resolution_clock::now();
            const auto elapsed =
                    std::chrono::duration_cast<std::chrono::seconds>(now - perf.last_print).count();
            if (elapsed >= 1) {
                perf.update_avg = (perf.update_time / perf.update_count);
                perf.draw_avg = (perf.draw_time / perf.draw_count);
                perf.update_time = 0.0;
                perf.update_count = 0;
                perf.draw_time = 0.0;
                perf.draw_count = 0;
                perf.last_print = now;
            }
            if (persistent_data.show_debug_window) {
                ImGui::Begin("Debug", &persistent_data.show_debug_window);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::Checkbox("Draw debug borders", &Control::DrawDebugBorders);
                ImGui::Text("UpdateRec average: %f ms", perf.update_avg);
                ImGui::Text("DrawRec average: %f ms", perf.draw_avg);
                ImGui::Separator();
                ImGui::Text("Mouse position %f, %f", GetMousePosition().x, GetMousePosition().y);
                ImGui::Text("Window size %i, %i", GetScreenWidth(), GetScreenHeight());
                ImGui::Text("Render size %i, %i", GetRenderWidth(), GetRenderHeight());
                ImGui::Text("DPI scale %f, %f", GetWindowScaleDPI().x, GetWindowScaleDPI().y);
                ImGui::Text("ImGui IO display size %f, %f", io.DisplaySize.x, io.DisplaySize.y);
                ImGui::Text("ImGui IO framebuffer scale %f, %f",
                            io.DisplayFramebufferScale.x,
                            io.DisplayFramebufferScale.y);
                const auto &logs = Logger::instance().entries();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.07f, 0.07f, 1.0f));
                ImGui::BeginChild("LogScroll",
                                  ImVec2(0, 0),
                                  true,
                                  ImGuiWindowFlags_HorizontalScrollbar);
                {
                    for (const std::string &line: logs) {
                        if (line.find("[ERROR]") != std::string::npos) {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 80, 80, 255));
                            ImGui::TextUnformatted(line.c_str());
                            ImGui::PopStyleColor();
                        } else if (line.find("[WARN]") != std::string::npos) {
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 80, 255));
                            ImGui::TextUnformatted(line.c_str());
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::TextUnformatted(line.c_str());
                        }
                    }
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                        ImGui::SetScrollHereY(1.0f);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::End();
            }
            ImGui::PopFont();
            rlImGuiEnd();
        }
        EndDrawing();
    };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop_wrapper, 0, 1);
#else
    while (!WindowShouldClose()) {
        loop_wrapper();
    }
#endif

    return 0;
}
