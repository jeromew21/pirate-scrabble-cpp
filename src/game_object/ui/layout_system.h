#pragma once

#include <memory>

#include "frameflow/layout.hpp"
#include "game_object/game_object.h"

namespace frameflow {
    struct System;
}

class LayoutSystem : public GameObject {
public:
    std::unique_ptr<frameflow::System> system;

    frameflow::NodeId root_node_id{frameflow::NullNode};

    bool fill_screen{true};

    LayoutSystem();

    void Update(float delta_time) override;

private:
    void AddChildHook(GameObject *child) override;
};
