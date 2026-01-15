#include "sprite.h"

void Sprite::Draw() {
    if (!texture) return;

    const auto gp = GlobalPosition();

    const auto width = static_cast<float>(texture->width);
    const auto height = static_cast<float>(texture->height);

    const Rectangle src = texture_region
                        ? *texture_region
                        : Rectangle{
                            0, 0, width, height
                        };

    const Rectangle dst = {gp.x, gp.y, target_dimensions_.x, target_dimensions_.y};

    DrawTexturePro(*texture, src, dst, {0, 0}, transform.rotation, WHITE);
}

void Sprite::SetTextureRegion(Rectangle r) {
    texture_region = r;
}

void Sprite::SetTexture(Texture2D new_texture, std::optional<float2> target_dimensions) {
    texture = new_texture;
    if (target_dimensions) {
        target_dimensions_ = *target_dimensions;
    } else {
        target_dimensions_ = {(float) new_texture.width, (float) new_texture.height};
    }
}
