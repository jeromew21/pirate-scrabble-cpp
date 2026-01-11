#include <string>
#include <chrono>
#include <iostream>
#include <algorithm>

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


using namespace frameflow;


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


void InitCrossPlatformWindow(int logicalWidth, int logicalHeight, const char *title) {
    // Try to disable hidpi
    constexpr unsigned int flags = FLAG_WINDOW_RESIZABLE
                                   | FLAG_MSAA_4X_HINT
                                   | FLAG_VSYNC_HINT;
    SetConfigFlags(flags);

    // Initialize with logical dimensions - raylib handles DPI internally
    InitWindow(logicalWidth, logicalHeight, title);

#ifdef __APPLE__
    // ideally this should be DPIScale, not hardcoded as 2, but whatever.
    auto dpi_scale = GetWindowScaleDPI();
    logicalWidth = logicalWidth / dpi_scale.x;
    logicalHeight = logicalHeight / dpi_scale.y;
    SetWindowSize(logicalWidth, logicalHeight);
#endif
}

bool Control::DrawDebugBorders = true;
float Tile::dim = 256.0;

std::unordered_map<char, RenderTexture2D> generate_tile_sprites(FT_Library ft) {
    // consider this just being an array...
    std::unordered_map<char, RenderTexture2D> tile_map;

    FT_Face face;
    if (FT_New_Face(ft, "arial.ttf", 0, &face)) {
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
            EndTextureMode();
        }
        EndTextureMode();
        tile_map[i] = tile_texture;
        Control::DrawDebugBorders = temp;
    }

    return tile_map;
}

int main() {
    // Make window resizable
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

    FT_Face face;
    if (FT_New_Face(ft, "arial.ttf", 0, &face)) {
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
    sprite->transform.rotation = 45;
    sprite->transform.local_position = {500, 500};


    /*
    System *ui = new System();
    NodeId root = add_generic(ui, NullNode);

    NodeId loading_node = add_center(ui, root);
    NodeId loading_text_node = add_generic(ui, loading_node);
    {
        get_node(ui, loading_node).anchors = {0, 0, 1, 1};
        get_node(ui, loading_text_node).anchors = {.5, 0.5, 0.5, 0.5};
        get_node(ui, loading_text_node).minimum_size = {400, 100};
    }

    NodeId multiplayer_node = add_box(ui, root, BoxData{Direction::Horizontal, Align::Start});
    get_node(ui, multiplayer_node).anchors = {0, 0, 1, 1};

    {
        NodeId left = add_generic(ui, multiplayer_node);
        get_node(ui, left).anchors = {0, 0, 0, 1};
        get_node(ui, left).minimum_size = {100, 0};
        get_node(ui, left).expand.x = 1;
    }

    std::vector<NodeId> tileIds;

    {
        NodeId middle = add_generic(ui, multiplayer_node);
        get_node(ui, middle).anchors = {0, 0, 0, 1};
        get_node(ui, middle).minimum_size = {48 * 10, 0};
        NodeId vbox = add_box(ui, middle, BoxData{Direction::Vertical, Align::Start});
        NodeId title = add_generic(ui, vbox);
        get_node(ui, title).minimum_size = {0, 200};
        get_node(ui, vbox).anchors = {0, 0, 1, 1};
        NodeId flow = add_flow(ui, vbox, FlowData{Direction::Horizontal, Align::Start});
        get_node(ui, flow).anchors = {0, 0, 1, 1};
        for (int i = 0; i < 144; i++) {
            NodeId tile = add_generic(ui, flow);
            get_node(ui, tile).minimum_size = {48, 48};
            tileIds.push_back(tile);
        }
    }

    {
        NodeId right = add_generic(ui, multiplayer_node);
        get_node(ui, right).anchors = {0, 0, 0, 1};
        get_node(ui, right).minimum_size = {100, 0};
        get_node(ui, right).expand.x = 1;
    }
    */

    double updateTime = 0.0;
    int updateCount = 0;
    double drawTime = 0.0;
    int drawCount = 0;
    auto lastPrint = std::chrono::high_resolution_clock::now();
    double updateAvg = 0;
    double drawAvg = 0;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(
#else
    while (!WindowShouldClose())
        [&]() {
#endif
            if (IsWindowResized()) {
                // maybe cancel tweens here...
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
                    ImGui::Begin("Debug");
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                                ImGui::GetIO().Framerate);
                    ImGui::Checkbox("Draw Debug Borders", &Control::DrawDebugBorders);
                    ImGui::Text("UpdateRec average: %f ms", updateAvg);
                    ImGui::Text("DrawRec average: %f ms", drawAvg);
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

#ifdef __EMSCRIPTEN__
        }, 0, 1);
#else
        }();
#endif

    // todo: shutdown harfbuzz/freetype
    std::cout << "Shutting down." << std::endl;

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
