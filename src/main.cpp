#include <string>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "raylib.h"
#include "rlImGui.h"

#include "imgui.h"
#include "misc/freetype/imgui_freetype.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "frameflow/layout.hpp"

#include "text/texthb.h"
#include "serialization/types.h"
#include "context/main_menu.h"
#include "context/multiplayer.h"
#include "game_object/entity/entity.h"
#include "game_object/entity/sprite.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "game_object/tween/tween.h"
#include "util/filesystem.h"
#include "util/logging/logging.h"
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

void InitCrossPlatformWindow(int logical_width, int logical_height, const char *title) {
    constexpr unsigned int flags = FLAG_WINDOW_RESIZABLE
                                   | FLAG_MSAA_4X_HINT
                                   | FLAG_VSYNC_HINT;
    SetConfigFlags(flags);

    // Initialize with logical dimensions - raylib handles DPI internally
    InitWindow(logical_width, logical_height, title);
    SetWindowSize(logical_width, logical_height);
#ifdef __EMSCRIPTEN__
    // Get the actual canvas size set by JavaScript
    int canvasWidth = EM_ASM_INT({return Module.canvas.width;});
    int canvasHeight = EM_ASM_INT({return Module.canvas.height;});

    // Tell raylib about the real size
    SetWindowSize(canvasWidth, canvasHeight);
#endif
}

FT_Face ft_load_font(const FT_Library &ft, const fs::path &path) {
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        Logger::instance().error("Failed to load font");
        exit(1);
    }
    return face;
}

FT_Library ft_init() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        Logger::instance().error("Failed to init FreeType");
        exit(1);
    }
    return ft;
}

std::unordered_map<char, RenderTexture2D> generate_tile_sprites(FT_Library ft) {
    // consider this just being an array...
    std::unordered_map<char, RenderTexture2D> tile_map;

    const auto face = ft_load_font(ft, FS_ROOT / "assets" / "arial.ttf");

    HBFont font(face, static_cast<int>(Tile::dim)); // pixel size 48

    LayoutSystem sys{};

    auto *tile = new Tile();
    sys.AddChild(tile);
    tile->Initialize();
    auto *label = new Label();
    tile->AddChild(label);
    label->font = &font;
    label->text = "A";
    label->color = BLACK;
    tile->GetNode()->bounds.origin = {0, 0};
    tile->GetNode()->bounds.size = tile->GetNode()->minimum_size;
    for (char i = 0; i < 127; i++) {
        label->text = i;
        tile->UpdateRec(0.016f);
        compute_layout(sys.system.get(), tile->node_id_);
        const RenderTexture2D tile_texture = LoadRenderTexture(
            static_cast<int>(tile->GetNode()->minimum_size.x),
            static_cast<int>(tile->GetNode()->minimum_size.y));
        const bool temp = Control::DrawDebugBorders;
        Control::DrawDebugBorders = false;
        BeginTextureMode(tile_texture);
        {
            ClearBackground({0, 0, 0, 0}); // IMPORTANT: alpha = 0dd
            tile->DrawRec();
        }
        EndTextureMode();
        tile_map[i] = tile_texture;
        Control::DrawDebugBorders = temp;
    }

    return tile_map;
}

float GetLogicalRatio() {
    return static_cast<float>(GetScreenWidth()) / static_cast<float>(GetRenderWidth());
}

int main() {
    // -------------------------
    // Initialize context
    // -------------------------
    GameObject root{};
    TweenManager tween_manager{};
    MainMenuContext menu_context{};
    root.AddChild(&menu_context);

    // -------------------------
    // Initialize raylib
    // -------------------------
    SetTraceLogLevel(LOG_NONE);
    SetExitKey(KEY_NULL);
    InitCrossPlatformWindow(menu_context.persistent_data.window_width,
                            menu_context.persistent_data.window_height,
                            "Pirate Scrabble");

    // -------------------------
    // Initialize ImGui
    // -------------------------
    rlImGuiSetup(true);
    const ImGuiIO &io = ImGui::GetIO();
    io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
    const auto ibm_plex_mono = FS_ROOT / "assets" / "IBM_Plex_Mono" / "IBMPlexMono-Light.ttf";
    ImFont *imgui_font = io.Fonts->AddFontFromFileTTF(ibm_plex_mono.c_str(),
                                                      32.0f*GetLogicalRatio());

    // -------------------------
    // Initialize FreeType
    // -------------------------
    auto *ft = ft_init();

    const auto arial = FS_ROOT / "assets" / "arial.ttf";
    const auto face = ft_load_font(ft, arial);

    HBFont font(face, 48); // pixel size 48

    auto tile_map = generate_tile_sprites(ft);

    // -------------------------
    // Initialize performance tracker
    // -------------------------
    Performance perf;
    perf.last_print = std::chrono::high_resolution_clock::now();

    main_loop_function = [&]() {
        if (IsWindowResized()) {
#ifdef __EMSCRIPTEN__
            int canvas_width = EM_ASM_INT({return Module.canvas.width;});
            int canvas_height = EM_ASM_INT({return Module.canvas.height;});
            SetWindowSize(canvas_width, canvas_height);
            Logger::instance().info("Resized to: {}, {}", canvas_width, canvas_height);
#else
            //SetWindowSize(, 800);
#endif
            menu_context.persistent_data.window_width = GetScreenWidth();
            menu_context.persistent_data.window_height = GetScreenHeight();
        }

        // Update
        {
            Profiler p("UpdateRec", perf.update_time, perf.update_count);
            const float dt = GetFrameTime();
            root.UpdateRec(dt);
            tween_manager.Update(dt);
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
            if (menu_context.persistent_data.show_debug_window) {
                ImGui::Begin("Debug", &menu_context.persistent_data.show_debug_window);
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

    // todo: shutdown harfbuzz/freetype
    Logger::instance().info("Shutting down");
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
