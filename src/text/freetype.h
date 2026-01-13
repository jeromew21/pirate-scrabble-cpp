#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

#include "util/filesystem/filesystem.h"

FT_Face ft_load_font(const FT_Library &ft, const fs::path &path);

FT_Library ft_init();
