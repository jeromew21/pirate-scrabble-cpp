#include <cfloat>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <thread>
#include <unordered_map>

#include <raylib.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <imgui.h>
#include <rlImGui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <frameflow/layout.hpp>

#include "util/util.h"
#include "text/texthb.h"
#include "serialization/types.h"
#include "context/sockets.h"

#include "context/main_menu.h"

using namespace frameflow;

static Color color_for(NodeType type) {
    switch (type) {
        case NodeType::Center: return BLUE;
        case NodeType::Box: return GREEN;
        case NodeType::Flow: return ORANGE;
        case NodeType::Generic: return RAYWHITE;
        default: return MAGENTA;
    }
}

static const char *node_type_name(NodeType type) {
    switch (type) {
        case NodeType::Center: return "Center";
        case NodeType::Box: return "Box";
        case NodeType::Flow: return "Flow";
        case NodeType::Generic: return "Generic";
        default: return "Unknown";
    }
}

static void DrawNodeRects(System &sys, NodeId id) {
    const Node &node = get_node(sys, id);

    Rectangle r{
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y
    };

    if (get_node(sys, id).parent != NullNode) {
        Color c = color_for(node.type);
        DrawRectangleLinesEx(r, 1.0f, c);

        // Draw node type text in top-left corner
        const char *type_str = node_type_name(node.type);
        int font_size = 12;
        DrawText(type_str,
                 static_cast<int>(node.bounds.origin.x) + 2,
                 static_cast<int>(node.bounds.origin.y) + 2,
                 font_size,
                 c);
    }

    for (NodeId child: node.children) {
        DrawNodeRects(sys, child);
    }
}

static Rectangle rl_rect(Rect r) {
    return {r.origin.x, r.origin.y, r.size.x, r.size.y};
}

int main() {
    // Make window resizable
    SetTraceLogLevel(LOG_NONE); //
    InitWindow(1920/2, 1080/2, "Pirate Scrabble");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    rlImGuiSetup(true); // sets up ImGui with either a dark or light default theme

    // set up contexts
    MainMenuContext menu_context{};

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

    System ui;
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

    while (!WindowShouldClose()) {
        {
            // Poll window size
            int win_width = GetScreenWidth();
            int win_height = GetScreenHeight();
            Node &root_node = get_node(ui, root);
            root_node.bounds.size = {float(win_width), float(win_height)};
            root_node.minimum_size = {float(win_width), float(win_height)};
        }

        // Compute layout
        compute_layout(ui, root);

        // Update
        menu_context.UpdateRec(GetFrameTime());

        // Draw
        BeginDrawing();
        {
            ClearBackground(DARKGRAY);

            rlImGuiBegin();


            menu_context.DrawRec();

            //DrawNodeRects(ui, loading_node);


            if (menu_context.state == MainMenuContext::State::InitialLoading) {
                Node node = get_node(ui, loading_text_node);
                std::string text = "Loading...";
                auto extents = MeasureTextHB(font, text);
                float2 offset{
                    (node.bounds.size.x - extents.width) * 0.5f,
                    (node.bounds.size.y - extents.height) * 0.5f
                };
                DrawTextHB(font, text, node.bounds.origin.x, node.bounds.origin.y + extents.ascent, BLACK);
            }

            /*
            DrawTextHB(font, "Hello, world!", 50, 300, BLACK);
            DrawTextHB(font, "f i ligatures!", 50, 360, RED);
            for (auto tile_id: tileIds) {
                auto node = get_node(ui, tile_id);
                Rectangle rec = rl_rect(node.bounds);
                DrawRectangleRoundedLinesEx(rec, 0.1, 5, 1, RED);
                auto extents = MeasureTextHB(font, "A");
                float2 offset{
                    (node.bounds.size.x - extents.width) * 0.5f,
                    (node.bounds.size.y - extents.height) * 0.5f
                };
                float2 text_pos = offset + node.bounds.origin;
                DrawTextHB(font, "A", text_pos.x, text_pos.y + extents.ascent, BLACK);
            }
            */

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
