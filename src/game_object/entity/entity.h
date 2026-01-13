#pragma once

#include "game_object/game_object.h"
#include "util/math.h"

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
