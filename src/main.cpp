#include <raylib.h>
#include <frameflow/layout.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <hb.h>
#include <hb-ft.h>

#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace frameflow;

// -------------------------
// Glyph struct for caching
// -------------------------
struct Glyph {
    Texture2D texture;
    int bitmap_left;
    int bitmap_top;
    int width;
    int height;
};

// -------------------------
// Font cache
// -------------------------
class HBFont {
public:
    FT_Face face;
    int pixelSize;
    std::unordered_map<unsigned int, Glyph> glyphs;

    HBFont(FT_Face f, int size) : face(f), pixelSize(size) {
        FT_Set_Pixel_Sizes(face, 0, pixelSize);
    }

    ~HBFont() {
        for (auto &pair : glyphs) UnloadTexture(pair.second.texture);
    }

    // Get or create glyph texture
    Glyph& GetGlyph(unsigned int glyphIndex) {
        auto it = glyphs.find(glyphIndex);
        if (it != glyphs.end()) return it->second;

        FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER);
        FT_GlyphSlot slot = face->glyph;

        unsigned int w = slot->bitmap.width;
        unsigned int h = slot->bitmap.rows;

        std::vector<Color> pixels(w * h, {255, 255, 255, 0});
        for (unsigned int row = 0; row < h; row++) {
            for (unsigned int col = 0; col < w; col++) {
                unsigned char v = slot->bitmap.buffer[row * w + col];
                pixels[row * w + col] = {255, 255, 255, v};
            }
        }

        Image img{};
        img.data = pixels.data();
        img.width = w;
        img.height = h;
        img.mipmaps = 1;
        img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        Texture2D tex = LoadTextureFromImage(img);
        //UnloadImage(img);

        Glyph glyph{tex, slot->bitmap_left, slot->bitmap_top, (int)w, (int)h};
        glyphs[glyphIndex] = glyph;
        return glyphs[glyphIndex];
    }
};

// -------------------------
// DrawTextHB function
// -------------------------
void DrawTextHB(HBFont &font, const std::string &text, float x, float y, Color tint) {
    hb_font_t* hb_font = hb_ft_font_create(font.face, NULL);
    hb_buffer_t* buf = hb_buffer_create();

    hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(buf);
    hb_shape(hb_font, buf, NULL, 0);

    unsigned int len = hb_buffer_get_length(buf);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, nullptr);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, nullptr);

    Vector2 pen = { x, y };

    for (unsigned int i = 0; i < len; i++) {
        Glyph &g = font.GetGlyph(info[i].codepoint);

        float drawX = pen.x + pos[i].x_offset / 64.0f + g.bitmap_left;
        float drawY = pen.y - pos[i].y_offset / 64.0f - g.bitmap_top;

        DrawTexture(g.texture, drawX, drawY, tint);

        pen.x += pos[i].x_advance / 64.0f;
        pen.y += pos[i].y_advance / 64.0f;
    }

    hb_buffer_destroy(buf);
    hb_font_destroy(hb_font);
}

static Color color_for(NodeType type) {
    switch (type) {
        case NodeType::Center:  return BLUE;
        case NodeType::Box:     return GREEN;
        case NodeType::Flow:    return ORANGE;
        case NodeType::Generic: return RAYWHITE;
        default:                return MAGENTA;
    }
}
static const char* node_type_name(NodeType type) {
    switch (type) {
        case NodeType::Center:  return "Center";
        case NodeType::Box:     return "Box";
        case NodeType::Flow:    return "Flow";
        case NodeType::Generic: return "Generic";
        default:                return "Unknown";
    }
}

static void DrawNodeRects(System& sys, NodeId id) {
    const Node& node = get_node(sys, id);

    Rectangle r {
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y
    };

    if (get_node(sys, id).parent != NullNode) {
        Color c = color_for(node.type);
        DrawRectangleLinesEx(r, 1.0f, c);

        // Draw node type text in top-left corner
        const char* type_str = node_type_name(node.type);
        int font_size = 12;
        DrawText(type_str,
                 static_cast<int>(node.bounds.origin.x) + 2,
                 static_cast<int>(node.bounds.origin.y) + 2,
                 font_size,
                 c);
    }

    for (NodeId child : node.children) {
        DrawNodeRects(sys, child);
    }
}

int main() {
    // Make window resizable
    InitWindow(1920, 1080, "Pirate Scrabble");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

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


    NodeId hbox = add_box(ui, root, BoxData{Direction::Horizontal, Align::Start});
    get_node(ui, hbox).anchors = {0, 0, 1, 1};

    {
        NodeId left = add_generic(ui, hbox);
        get_node(ui, left).anchors = {0, 0, 0, 1};
        get_node(ui, left).minimum_size = {100, 0};
        get_node(ui, left).expand.x = 1;
    }

    {
        NodeId middle = add_generic(ui, hbox);
        get_node(ui, middle).anchors = {0, 0, 0, 1};
        get_node(ui, middle).minimum_size = {48*10, 0};
        NodeId vbox = add_box(ui, middle, BoxData{Direction::Vertical, Align::Start});
        NodeId title = add_generic(ui, vbox);
        get_node(ui, title).minimum_size = {0, 200};
        get_node(ui, vbox).anchors = {0, 0, 1, 1};
        NodeId flow = add_flow(ui, vbox, FlowData{Direction::Horizontal, Align::Start});
        get_node(ui, flow).anchors = {0, 0, 1, 1};
        for (int i = 0; i < 144; i++) {
            NodeId tile = add_generic(ui, flow);
            get_node(ui, tile).minimum_size = {48, 48};
        }
    }

    {
        NodeId right = add_generic(ui, hbox);
        get_node(ui, right).anchors = {0, 0, 0, 1};
        get_node(ui, right).minimum_size = {100, 0};
        get_node(ui, right).expand.x = 1;
    }

    while (!WindowShouldClose()) {
        // Poll window size
        int win_width = GetScreenWidth();
        int win_height = GetScreenHeight();

        // Resize root node to match window
        Node& root_node = get_node(ui, root);
        root_node.bounds.size = { float(win_width), float(win_height) };
        root_node.minimum_size = { float(win_width), float(win_height) };

        // Compute layout
        compute_layout(ui, root);

        // Draw
        BeginDrawing();
        ClearBackground(DARKGRAY);

        DrawNodeRects(ui, root);

        DrawTextHB(font, "Hello, world!", 50, 300, BLACK);
        DrawTextHB(font, "f i ligatures!", 50, 360, RED);


        EndDrawing();
    }

    CloseWindow();
    return 0;
}

