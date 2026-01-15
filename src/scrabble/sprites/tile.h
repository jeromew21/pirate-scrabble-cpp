#pragma once

#include <cassert>

#include "game_object/ui/control.h"
#include "text/freetype_library.h"
#include "util/math.h"
#include "util/logging/logging.h"



class Tile : public CenterContainer {
public:
    static inline float dim = 81.0;

    static void InitializeTextures(FT_Library ft);

    static void DeInitializeTextures();

    static RenderTexture2D GetTileTexture();

    static float2 GetTileTextureRegion(char c);

    void Initialize() const;

    void Draw() override;
};
