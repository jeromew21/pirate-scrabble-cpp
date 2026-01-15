#include "tile.h"

#include <unordered_map>
#include <memory>

#include "raylib.h"

#include "game_object/ui/drawing.h"
#include "text/freetype_library.h"
#include "text/texthb.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"

namespace {
    std::unique_ptr<std::unordered_map<char, RenderTexture2D> > map_;

    void generate_tile_sprites(FT_Library ft, std::unordered_map<char, RenderTexture2D> &tile_map) {
        const auto face = ft_load_font(ft, FS_ROOT / "assets" / "arial.ttf");

        HBFont font(face, static_cast<int>(Tile::dim)); // pixel size 48

        auto *sys = new LayoutSystem();

        auto *tile = new Tile();
        sys->AddChild(tile);
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
            compute_layout(sys->system.get(), tile->node_id_);
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
        ft_face_de_init(face);
        sys->Delete();
    }
}

void Tile::Initialize() const {
    GetNode()->minimum_size = {dim, dim};
}

void Tile::DeInitializeTextures() {
    for (auto &pair: *map_) {
        UnloadRenderTexture(pair.second);
    }
}

void Tile::Draw() {
    const Color color = {243, 237, 166, 255};
    const auto node = *GetNode();
    DrawRoundedRectangle(
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y,
        (3.0f / 48.0f) * dim, color);
}

// TODO: make this a spritesheet, single texturew
void Tile::InitializeTextures(FT_Library ft) {
    map_ = std::make_unique<std::unordered_map<char, RenderTexture2D> >();
    generate_tile_sprites(ft, *map_);
}

RenderTexture2D Tile::GetTileTexture(const char c) {
    return (*map_)[c];
}
