#pragma once

#include <optional>

#include "raylib.h"

#include "entity.h"

class Sprite : public Entity {
public:
    std::optional<Texture2D> texture;

    void Draw() override;

    void SetTextureRegion(Rectangle r);

    void SetTexture(Texture2D new_texture, std::optional<float2> target_dimensions = std::nullopt);
private:
    float2 target_dimensions_{};
    std::optional<Rectangle> texture_region;
};
