#include "entity.h"

#include <raylib.h>

float2 Entity::GlobalPosition() {
    return transform.local_position;
}

void Sprite::Draw() {
    if (texture == nullptr) return;

    auto gp = GlobalPosition();

    float width = (float)texture->width;
    float height = (float)texture->height;

    Rectangle src = {
        0, 0,width, height
    };

    Rectangle dst = {gp.x, gp.y, width, height};

    DrawTexturePro(*texture, src, dst, {width/2, height/2}, transform.rotation, WHITE);
}

void Sprite::SetTexture(Texture2D *new_texture) {
    texture = new_texture;
}
