#pragma once

#include "ft2build.h"
#include "freetype/internal/ftobjs.h"

#include FT_FREETYPE_H

#include "util/filesystem/filesystem.h"

FT_Face ft_load_font(const FT_Library &ft, const fs::path &path);

FT_Library ft_init();

void ft_de_init(FT_Library ft);

void ft_face_de_init(FT_Face face);
