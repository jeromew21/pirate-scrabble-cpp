#include "texthb.h"

#include <cfloat>
#include <vector>
#include <unordered_map>

#include "hb.h"
#include "hb-ft.h"

#include "freetype_library.h"

// -------------------------
// Glyph struct for caching
// -------------------------
struct Glyph {
    Texture2D texture;
    int bitmap_left;
    int bitmap_top;
    int width;
    int height;
};

// -------------------------
// Font cache
// -------------------------
struct HBFont::Impl {
    FT_Face face;
    int pixelSize;
    std::unordered_map<unsigned int, Glyph> glyphs;

    Impl(FT_Face f, int size) : face(f), pixelSize(size) {
        FT_Set_Pixel_Sizes(face, 0, pixelSize);
    }

    ~Impl() {
        for (auto &pair : glyphs) UnloadTexture(pair.second.texture);
    }

    // Get or create glyph texture
    Glyph& GetGlyph(unsigned int glyphIndex) {
        auto it = glyphs.find(glyphIndex);
        if (it != glyphs.end()) return it->second;

        FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER);
        FT_GlyphSlot slot = face->glyph;

        unsigned int w = slot->bitmap.width;
        unsigned int h = slot->bitmap.rows;

        std::vector<Color> pixels(w * h, {255, 255, 255, 0});
        for (unsigned int row = 0; row < h; row++) {
            for (unsigned int col = 0; col < w; col++) {
                unsigned char v = slot->bitmap.buffer[row * w + col];
                pixels[row * w + col] = {255, 255, 255, v};
            }
        }

        Image img{};
        img.data = pixels.data();
        img.width = w;
        img.height = h;
        img.mipmaps = 1;
        img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        Texture2D tex = LoadTextureFromImage(img);
        //UnloadImage(img); //TODO: unload this?

        Glyph glyph{tex, slot->bitmap_left, slot->bitmap_top, (int)w, (int)h};
        glyphs[glyphIndex] = glyph;
        return glyphs[glyphIndex];
    }
};


HBFont::HBFont(FT_Face f, int size) : impl_(std::make_unique<Impl>(f, size)) {}

HBFont::~HBFont() = default;

HBFont::HBFont(HBFont&&) noexcept = default;

HBFont& HBFont::operator=(HBFont&&) noexcept = default;

void DrawTextHB(HBFont &font, const std::string &text, float x, float y, Color tint) {
    hb_font_t* hb_font = hb_ft_font_create(font.impl_->face, NULL);
    hb_buffer_t* buf = hb_buffer_create();

    hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(buf);
    hb_shape(hb_font, buf, NULL, 0);

    unsigned int len = hb_buffer_get_length(buf);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, nullptr);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, nullptr);

    Vector2 pen = { x, y };

    for (unsigned int i = 0; i < len; i++) {
        Glyph &g = font.impl_->GetGlyph(info[i].codepoint);

        float drawX = pen.x + pos[i].x_offset / 64.0f + g.bitmap_left;
        float drawY = pen.y - pos[i].y_offset / 64.0f - g.bitmap_top;

        DrawTexture(g.texture, drawX, drawY, tint);

        pen.x += pos[i].x_advance / 64.0f;
        pen.y += pos[i].y_advance / 64.0f;
    }

    hb_buffer_destroy(buf);
    hb_font_destroy(hb_font);
}

// -------------------------
// Measure text with cached HBFont
// -------------------------
TextMetrics MeasureTextHB(HBFont &font, const std::string &text) {
    hb_font_t* hb_font = hb_ft_font_create(font.impl_->face, nullptr);
    hb_buffer_t* buf = hb_buffer_create();

    hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);
    hb_buffer_guess_segment_properties(buf);
    hb_shape(hb_font, buf, nullptr, 0);

    unsigned int len = hb_buffer_get_length(buf);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buf, nullptr);
    hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, nullptr);

    float width = 0;
    float minY = FLT_MAX, maxY = -FLT_MAX;

    for (unsigned int i = 0; i < len; i++) {
        Glyph &g = font.impl_->GetGlyph(info[i].codepoint);

        float top = -g.bitmap_top;
        float bottom = -g.bitmap_top + g.height;

        if (top < minY) minY = top;
        if (bottom > maxY) maxY = bottom;

        width += pos[i].x_advance / 64.0f;
    }

    hb_buffer_destroy(buf);
    hb_font_destroy(hb_font);

    return { width, maxY - minY, -minY, maxY };
}
