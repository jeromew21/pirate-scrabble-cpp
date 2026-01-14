#include "sprite.h"

void Sprite::Draw() {
    if (!texture.has_value()) return;

    const auto gp = GlobalPosition();

    const auto width = static_cast<float>(texture->width);
    const auto height = static_cast<float>(texture->height);

    Rectangle src = {
        0, 0, width, -height
    };

    Rectangle dst = {gp.x, gp.y, width, height};

    DrawTexturePro(*texture, src, dst, {width/2, height/2}, transform.rotation, WHITE);
}

void Sprite::SetTexture(Texture2D new_texture) {
    texture = new_texture;
}
