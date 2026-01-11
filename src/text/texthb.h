#pragma once

#include <string>
#include <memory>

#include <raylib.h>

#include <ft2build.h>
#include FT_FREETYPE_H


// -------------------------
// TextMetrics struct
// -------------------------
struct TextMetrics {
    float width;
    float height;
    float ascent;
    float descent;
};

class HBFont {
public:
    HBFont(FT_Face f, int size);                  // defined in .cpp
    ~HBFont();                 // must be defined where Impl is complete

    HBFont(HBFont&&) noexcept;    // movable
    HBFont& operator=(HBFont&&) noexcept;

    HBFont(const HBFont&) = delete;
    HBFont& operator=(const HBFont&) = delete;

    struct Impl;            // forward declaration only
    std::unique_ptr<Impl> impl_;
};

// -------------------------
// DrawTextHB function
// -------------------------
void DrawTextHB(HBFont &font, const std::string &text, float x, float y, Color tint);

// -------------------------
// Measure text with cached HBFont
// -------------------------
TextMetrics MeasureTextHB(HBFont &font, const std::string &text);
