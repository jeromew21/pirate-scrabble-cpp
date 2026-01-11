#pragma once

#include <memory>

#include "game_object/game_object.h"

namespace frameflow {
    struct System;
}

class LayoutSystem : public GameObject {
public:
    std::unique_ptr<frameflow::System> system_;

    explicit LayoutSystem();

    void Update(float delta_time) override;

    bool fill_screen{true};

private:

    void AddChildHook(GameObject *child) override;
};
