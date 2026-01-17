#pragma once

#include "game_object/ui/control.h"
#include "text/freetype_library.h"
#include "util/math.h"

namespace scrabble {
    class Tile : public CenterContainer {
    public:
        static inline float dim = 81.0;

        static void InitializeTextures();

        static void DeInitializeTextures();

        static RenderTexture2D GetTileTexture();

        static float2 GetTileTextureRegion(char c);

        void Draw() override;
    };
}
