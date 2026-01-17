#include "tile.h"

#include <unordered_map>
#include <memory>

#include "raylib.h"
#include "fmt/color.h"

#include "game_object/ui/drawing.h"
#include "text/texthb.h"
#include "game_object/ui/control.h"
#include "game_object/ui/layout_system.h"
#include "util/logging/logging.h"

using namespace scrabble;

namespace {
    constexpr float RENDER_SIZE = 256.f;
    float2 map_[128];
    RenderTexture2D tile_texture_map;

    void generate_tile_sprites() {
        const float prev_dim = Tile::dim;
        Tile::dim = RENDER_SIZE;
        const auto face = Freetype_Face(FS_ROOT / "assets" / "arial.ttf");

        HBFont font(face, static_cast<int>(Tile::dim * 0.85f)); // pixel size 48

        auto *sys = new LayoutSystem();

        auto *tile = new Tile();
        sys->AddChild(tile);
        auto *label = new Label();
        tile->AddChild(label);
        label->font = &font;
        label->text = "A";
        label->color = BLACK;
        tile->GetNode()->bounds.origin = {0, 0};
        tile->GetNode()->bounds.size = {Tile::dim, Tile::dim};
        tile->GetNode()->minimum_size = {Tile::dim, Tile::dim};
        BeginTextureMode(tile_texture_map);
        ClearBackground({0, 0, 0, 0}); // IMPORTANT: alpha = 0dd
        EndTextureMode();
        const bool temp = Control::DrawDebugBorders;
        Control::DrawDebugBorders = false;
        for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 8; col++) {
                const char code = static_cast<char>(row * 8 + col);
                tile->GetNode()->bounds.origin = {
                    static_cast<float>(col) * Tile::dim, Tile::dim * (15.0f - static_cast<float>(row))
                };
                label->text = std::string(1, code);
                tile->UpdateRec(0.016f);
                compute_layout(sys->system.get(), tile->node_id_);
                BeginTextureMode(tile_texture_map);
                {
                    tile->DrawRec();
                }
                EndTextureMode();
                map_[code] = {static_cast<float>(col) * Tile::dim, static_cast<float>(row) * Tile::dim};
            }
        }
        Control::DrawDebugBorders = temp;
        sys->Delete();
        Tile::dim = prev_dim;
    }

    class RoundedRectShader {
        Shader shader{};
        Texture2D blankTex{};
        int sizeLoc, radiusLoc, colorLoc;

    public:
        RoundedRectShader() {
            const char *fragment_shader_code =
#if defined(PLATFORM_WEB)
                            R"(#version 100
precision mediump float;

varying vec2 fragTexCoord;

uniform vec2 size;
uniform float radius;
uniform vec4 rectColor;

float sdRoundedRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    vec2 pixelPos = fragTexCoord * size;
    vec2 centerPos = pixelPos - size * 0.5;

    float dist = sdRoundedRect(centerPos, size * 0.5, radius);
    float alpha = smoothstep(1.0, -1.0, dist);

    gl_FragColor = vec4(rectColor.rgb, rectColor.a * alpha);
}
)"
#else
                            R"(#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform vec2 size;
uniform float radius;
uniform vec4 rectColor;

float sdRoundedRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    vec2 pixelPos = fragTexCoord * size;
    vec2 centerPos = pixelPos - size * 0.5;

    float dist = sdRoundedRect(centerPos, size * 0.5, radius);
    float alpha = smoothstep(1.0, -1.0, dist);

    finalColor = vec4(rectColor.rgb, rectColor.a * alpha);
}
)"
#endif
                    ;
            shader = LoadShaderFromMemory(nullptr, fragment_shader_code);
            // Check if shader loaded
            if (shader.id == 0) {
                Logger::instance().info("Failed to compile shader");
                // Check browser console for WebGL shader compilation errors
                exit(1);
            }
            sizeLoc = GetShaderLocation(shader, "size");
            radiusLoc = GetShaderLocation(shader, "radius");
            colorLoc = GetShaderLocation(shader, "rectColor");

            // Create a 1x1 white texture
            Image img = GenImageColor(1, 1, WHITE);
            blankTex = LoadTextureFromImage(img);
            UnloadImage(img);
        }

        void Draw(float x, float y, float width, float height, float radius, Color color) const {
            BeginShaderMode(shader);

            const float size[2] = {(float) width, (float) height};
            SetShaderValue(shader, sizeLoc, size, SHADER_UNIFORM_VEC2);
            SetShaderValue(shader, radiusLoc, &radius, SHADER_UNIFORM_FLOAT);
            const float col[4] = {
                static_cast<float>(color.r) / 255.0f,
                static_cast<float>(color.g) / 255.0f,
                static_cast<float>(color.b) / 255.0f,
                static_cast<float>(color.a) / 255.0f
            };
            SetShaderValue(shader, colorLoc, col, SHADER_UNIFORM_VEC4);

            // Use DrawTexturePro to ensure proper UVs
            DrawTexturePro(
                blankTex,
                {0, 0, 1, 1}, // Source (whole 1x1 texture)
                {x, y, width, height}, // Destination
                {0, 0},
                0,
                WHITE
            );

            EndShaderMode();
        }

        ~RoundedRectShader() {
            UnloadShader(shader);
            UnloadTexture(blankTex);
        }
    };

    std::unique_ptr<RoundedRectShader> rounded_rect_shader;
}

void Tile::DeInitializeTextures() {
    UnloadRenderTexture(tile_texture_map);
}

void Tile::Draw() {
    constexpr Color color = {243, 237, 166, 255};
    const auto node = *GetNode();
    rounded_rect_shader->Draw(
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y,
        (4.0f / 48.0f) * dim, color);
}

// TODO: make this a spritesheet, single texture
void Tile::InitializeTextures() {
    rounded_rect_shader = std::make_unique<RoundedRectShader>();
    tile_texture_map = LoadRenderTexture(RENDER_SIZE * 8, RENDER_SIZE * 16);
    SetTextureFilter(tile_texture_map.texture, TEXTURE_FILTER_BILINEAR);
    generate_tile_sprites();
}

RenderTexture2D Tile::GetTileTexture() {
    return tile_texture_map;
}

float2 Tile::GetTileTextureRegion(const char c) {
    return map_[c];
}
