#include <string>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <raylib.h>
#include <rlImGui.h>

#include <fmt/core.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <frameflow/layout.hpp>

#include "imgui.h"
#include "text/texthb.h"
#include "serialization/types.h"
#include "context/main_menu.h"
#include "game_object/entity/entity.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "game_object/tween/tween.h"
#include "scrabble/tile.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace fs = std::filesystem;

static std::function<void()> main_loop_function;

extern "C" void loop_wrapper() {
    main_loop_function();
}

using namespace frameflow;

#ifdef __EMSCRIPTEN__
fs::path FS_ROOT = "/";
#else
fs::path FS_ROOT = ".";
#endif

struct Profiler {
    const char *name;
    double &accumulator;
    int &count;
    std::chrono::high_resolution_clock::time_point start;

    Profiler(const char *n, double &acc, int &c)
        : name(n), accumulator(acc), count(c), start(std::chrono::high_resolution_clock::now()) {
    }

    ~Profiler() {
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration<double, std::milli>(end - start).count();
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
#ifdef __EMSCRIPTEN__
    // Get the actual canvas size set by JavaScript
    int canvasWidth = EM_ASM_INT({
        return Module.canvas.width;
    });
    int canvasHeight = EM_ASM_INT({
        return Module.canvas.height;
    });

    // Tell raylib about the real size
    SetWindowSize(canvasWidth, canvasHeight);

    std::cout << "Synced raylib to canvas size: " << canvasWidth << "x" << canvasHeight << std::endl;
#endif

#ifdef __APPLE__
    auto dpi_scale = GetWindowScaleDPI();
    logical_width = logical_width / dpi_scale.x;
    logical_height = logical_height / dpi_scale.y;
    SetWindowSize(logical_width, logical_height);
#endif
}

bool Control::DrawDebugBorders = true;

float Tile::dim = 256.0;

std::unordered_map<char, RenderTexture2D> generate_tile_sprites(const FT_Library ft) {
    // consider this just being an array...
    std::unordered_map<char, RenderTexture2D> tile_map;

    const auto arial = FS_ROOT / "assets" / "arial.ttf";
    FT_Face face;
    if (FT_New_Face(ft, arial.c_str(), 0, &face)) {
        std::cerr << "Failed to load font\n";
        exit(67);
    }

    auto black = BLACK;

    HBFont font(face, static_cast<int>(Tile::dim)); // pixel size 48

    LayoutSystem sys{};

    auto *tile = new Tile();
    sys.AddChild(tile);
    tile->Initialize();
    auto *c = new Label();
    tile->AddChild(c);
    c->font = &font,
            c->text = "A";
    c->color = &black;
    tile->GetNode()->bounds.origin = {0, 0};
    tile->GetNode()->bounds.size = tile->GetNode()->minimum_size;
    for (char i = 0; i < 127; i++) {
        c->text = i;
        tile->UpdateRec(0.016f);
        compute_layout(sys.system.get(), tile->node_id_);
        // can we draw this to a render texture?
        const RenderTexture2D tile_texture = LoadRenderTexture(
            tile->GetNode()->minimum_size.x,
            tile->GetNode()->minimum_size.y);
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

int main() {
    SetTraceLogLevel(LOG_NONE);
    InitCrossPlatformWindow(1920, 1080, "Pirate Scrabble");

    rlImGuiSetup(true); // sets up ImGui with either a dark or light default theme

    // -------------------------
    // Initialize FreeType
    // -------------------------
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Failed to init FreeType\n";
        return 1;
    }

    const auto arial = FS_ROOT / "assets" / "arial.ttf";
    FT_Face face;
    if (FT_New_Face(ft, arial.c_str(), 0, &face)) {
        std::cerr << "Failed to load font\n";
        return 1;
    }

    HBFont font(face, 48); // pixel size 48

    TweenManager tween_manager;

    GameObject root{};

    auto tile_map = generate_tile_sprites(ft);

    // set up contexts
    MainMenuContext menu_context{};
    root.AddChild(&menu_context);

    auto black = BLACK;

    LayoutSystem sys{};
    root.AddChild(&sys);
    {
        auto *login_screen = new CenterContainer();
        sys.AddChild(login_screen);
        login_screen->GetNode()->anchors = {0, 0, 1, 1};
        auto *login_box = new BoxContainer(BoxData{Direction::Horizontal, Align::Start});
        login_screen->AddChild(login_box);
        login_box->GetNode()->minimum_size = {500, 500};
        auto *label = new LineInput();
        login_box->AddChild(label);
        label->font = &font;
        label->text = "Hello world";
        label->color = &black;
    }

    auto *sprite = new Sprite();
    sprite->SetTexture(&tile_map['C'].texture);
    root.AddChild(sprite);
    sprite->transform.local_position = {500, 500};

    double updateTime = 0.0;
    int updateCount = 0;
    double drawTime = 0.0;
    int drawCount = 0;
    auto lastPrint = std::chrono::high_resolution_clock::now();
    double updateAvg = 0;
    double drawAvg = 0;

    main_loop_function = [&]() {
        if (IsWindowResized()) {
#ifdef __EMSCRIPTEN__
            // Sync raylib with the new canvas size
            int canvasWidth = EM_ASM_INT({
                return Module.canvas.width;
            });
            int canvasHeight = EM_ASM_INT({
                return Module.canvas.height;
            });

            SetWindowSize(canvasWidth, canvasHeight);
            std::cout << "Resized to: " << canvasWidth << "x" << canvasHeight << std::endl;
#else
            //SetWindowSize(, 800);
#endif
        }

        // Update
        {
            Profiler p("UpdateRec", updateTime, updateCount);
            const float dt = GetFrameTime();
            root.UpdateRec(dt);
            tween_manager.Update(dt);
        }

        // Draw
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);
            rlImGuiBegin();
            {
                Profiler p("DrawRec", drawTime, drawCount);
                root.DrawRec();
            }
            {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastPrint).count();
                if (elapsed >= 1) {
                    updateAvg = (updateTime / updateCount);
                    drawAvg = (drawTime / drawCount);

                    // reset counters
                    updateTime = 0.0;
                    updateCount = 0;
                    drawTime = 0.0;
                    drawCount = 0;
                    lastPrint = now;
                }
                const ImGuiIO& io = ImGui::GetIO();
                ImGui::Begin("Debug");
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);
                ImGui::Checkbox("Draw Debug Borders", &Control::DrawDebugBorders);
                ImGui::Text("UpdateRec average: %f ms", updateAvg);
                ImGui::Text("DrawRec average: %f ms", drawAvg);
                ImGui::Text("Mouse position %i, %i", (int)GetMousePosition().x,(int) GetMousePosition().y);
                ImGui::Text("Window size %i, %i", (int)GetScreenWidth(),(int) GetScreenHeight());
                ImGui::Text("Render size %i, %i", (int)GetRenderWidth(),(int) GetRenderHeight());
                ImGui::Text("DPI scale %i, %i", (int)GetWindowScaleDPI().x,(int) GetWindowScaleDPI().y);
                ImGui::Text("ImGui display size %f, %f", io.DisplaySize.x, io.DisplaySize.y);
                ImGui::Text("ImGui IO framebuffer scale %f, %f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
                if (ImGui::Button("Spin")) {
                    tween_manager.CreateTweenFromTo(
                        &sprite->transform.rotation,
                        0,
                        360,
                        1,
                        Easing::EaseInOutSine);
                }
                ImGui::End();
            }
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
    std::cout << "Shutting down." << std::endl;

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
