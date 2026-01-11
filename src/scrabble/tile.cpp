#include "tile.h"

#include "raylib.h"

#include "game_object/ui/drawing.h"

void Tile::Initialize() const {
    GetNode()->minimum_size = {dim, dim};
}

void Tile::Draw() {
    const Color color = {243, 237, 166, 255};
    const auto node = *GetNode();
    DrawRoundedRectangle(
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y,
        (3.0f/48.0f)*dim, color);
}
