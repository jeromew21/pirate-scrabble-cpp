#pragma once

#include "game_object/ui/control.h"

class Tile : public CenterContainer {
public:
    static constexpr float dim = 64;
    void Initialize() const;
    void Draw() override;
};
