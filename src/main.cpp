#include <string>
#include <vector>
#include <iostream>

#include <raylib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <rlImGui.h>

#include <frameflow/layout.hpp>

#include "text/texthb.h"
#include "serialization/types.h"
#include "context/main_menu.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"

using namespace frameflow;


static Rectangle rl_rect(Rect r) {
    return {r.origin.x, r.origin.y, r.size.x, r.size.y};
}

int main() {
    // Make window resizable
    SetTraceLogLevel(LOG_NONE); //
    InitWindow(1920, 1080, "Pirate Scrabble");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

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

    GameObject root{};

    // set up contexts
    MainMenuContext menu_context{};
    root.AddChild(&menu_context);

    LayoutSystem sys;
    root.AddChild(&sys);
    {
        auto login_screen = new CenterContainer();
        sys.AddChild(login_screen);
        auto login_box = new Control();
        login_screen->AddChild(login_box);
        login_box->GetNode()->minimum_size = {100, 100};
    }


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

    while (!WindowShouldClose()) {

        // Update
        root.UpdateRec(GetFrameTime());

        // Draw
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);

            rlImGuiBegin();

            root.DrawRec();

            rlImGuiEnd();
        }
        EndDrawing();
    }
    // todo: shutdown harfbuzz/freetype
    std::cout << "Shutting down." << std::endl;

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
