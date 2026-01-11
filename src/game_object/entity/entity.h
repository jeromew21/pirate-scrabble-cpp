#pragma once

#include "game_object/game_object.h"
#include "util/util.h"

#include <raylib.h>

class Entity : public GameObject {
public:
    struct Transform {
        float2 local_position;
        float rotation;
        float2 scale;
    };

    Transform transform;

    float2 GlobalPosition();
};

class Sprite : public Entity {
public:
    Texture2D *texture{nullptr};

    void Draw() override;

    void SetTexture(Texture2D *new_texture);
};
