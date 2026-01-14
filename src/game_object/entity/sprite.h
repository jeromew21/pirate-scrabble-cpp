#pragma once

#include <optional>

#include "raylib.h"

#include "entity.h"

class Sprite : public Entity {
public:
    std::optional<Texture2D> texture;

    void Draw() override;

    void SetTexture(Texture2D new_texture);
};
