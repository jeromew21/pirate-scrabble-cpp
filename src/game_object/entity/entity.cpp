#include "entity.h"

#include "raylib.h"

float2 Entity::GlobalPosition() {
    return transform.local_position;
}

