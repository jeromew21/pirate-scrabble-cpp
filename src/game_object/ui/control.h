#pragma once

#include <cstdint>

#include "frameflow/layout.hpp"
#include "game_object/game_object.h"

class LayoutSystem;

class Control : public GameObject {
public:
    LayoutSystem *layout_system_{nullptr};

    explicit Control() = default;

    virtual void InitializeLayout(LayoutSystem *system);

    frameflow::NodeId node_id_{frameflow::NullNode};

    frameflow::Node *GetNode() const;

    void Draw() override;

private:
    void AddChildHook(GameObject *child) override;
};

class BoxContainer : public Control {
public:
    explicit BoxContainer(const frameflow::BoxData &data);
    void InitializeLayout(LayoutSystem *system) override;
    frameflow::BoxData box_data;
};

class CenterContainer : public Control {
public:
    explicit CenterContainer() = default;
    void InitializeLayout(LayoutSystem *system) override;
};
