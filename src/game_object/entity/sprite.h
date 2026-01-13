#pragma once

#include "entity.h"
#include "raylib.h"

class Sprite : public Entity {
public:
    Texture2D *texture{nullptr};

    void Draw() override;

    void SetTexture(Texture2D *new_texture);
};
