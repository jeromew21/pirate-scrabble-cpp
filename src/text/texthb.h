#pragma once

#include <string>
#include <memory>

#include "util/filesystem/filesystem.h"

// -------------------------
// TextMetrics struct
// -------------------------
struct TextMetrics {
    float width;
    float height;
    float ascent;
    float descent;
};

class Freetype_Face;

struct Color;

class HBFont {
public:
    HBFont(const Freetype_Face &f, int size); // defined in .cpp
    ~HBFont(); // must be defined where Impl is complete

    HBFont(HBFont &&) noexcept; // movable
    HBFont &operator=(HBFont &&) noexcept;

    HBFont(const HBFont &) = delete;

    HBFont &operator=(const HBFont &) = delete;

    friend void DrawTextHB(HBFont &font, const std::string &text, float x, float y, const Color &tint);

    friend TextMetrics MeasureTextHB(HBFont &font, const std::string &text);

private:
    struct Impl; // forward declaration only
    std::unique_ptr<Impl> impl_;
};

class Freetype_Face {
public:
    explicit Freetype_Face(const fs::path &path);

    ~Freetype_Face();

    friend void ft_face_de_init(const Freetype_Face &face);

    friend Freetype_Face ft_load_font(const fs::path &path);

    friend HBFont::HBFont(const Freetype_Face &f, int size); // defined in .cpp

private:
    struct Impl;
    //std::unique_ptr<Impl> impl_;
    Impl* impl_;
};

void fonts_init();

void fonts_de_init();

void ft_face_de_init(const Freetype_Face &face);

// -------------------------
// DrawTextHB function
// -------------------------
void DrawTextHB(HBFont &font, const std::string &text, float x, float y, const Color &tint);

// -------------------------
// Measure text with cached HBFont
// -------------------------
TextMetrics MeasureTextHB(HBFont &font, const std::string &text);
