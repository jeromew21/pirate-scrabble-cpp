#pragma once

#include "game_object/ui/control.h"
#include "text/freetype_library.h"

class Tile : public CenterContainer {
public:
    static inline float dim = 81.0;

    static void InitializeTextures(FT_Library ft);

    static RenderTexture2D GetTileTexture(char c);

    void Initialize() const;

    void Draw() override;
};
