#include "freetype_library.h"

#include "util/logging/logging.h"

FT_Face ft_load_font(const FT_Library &ft, const fs::path &path) {
    FT_Face face;
    if (FT_New_Face(ft, path.string().c_str(), 0, &face)) {
        Logger::instance().error("Failed to load font");
        exit(1);
    }
    return face;
}

FT_Library ft_init() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        Logger::instance().error("Failed to init FreeType");
        exit(1);
    }
    return ft;
}

void ft_de_init(FT_Library ft) {
    FT_Done_Library(ft);
}

void ft_face_de_init(FT_Face face) {
    FT_Done_Face(face);
}
