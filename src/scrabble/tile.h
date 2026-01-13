#pragma once

#include "game_object/ui/control.h"

class Tile : public CenterContainer {
public:
    static float dim;
    void Initialize() const;
    void Draw() override;
};
