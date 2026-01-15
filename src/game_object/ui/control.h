#pragma once

#include <string>

#include "raylib.h"
#include "frameflow/layout.hpp"
#include "game_object/game_object.h"

class LayoutSystem;

class Control : public GameObject {
public:
    static inline bool DrawDebugBorders = true;

    LayoutSystem *layout_system_{nullptr};

    frameflow::NodeId node_id_{frameflow::NullNode};

    Control() = default;

    virtual void InitializeLayout(LayoutSystem *system);

    void ForceComputeLayout() const;

    [[nodiscard]] frameflow::Node *GetNode() const;

    void Draw() override;

private:
    void AddChildHook(GameObject *child) override;

    void DeleteHook() override;
};

class BoxContainer : public Control {
public:
    frameflow::BoxData box_data;

    explicit BoxContainer(const frameflow::BoxData &data);

    void InitializeLayout(LayoutSystem *system) override;
};

class FlowContainer : public Control {
public:
    frameflow::FlowData flow_data;

    explicit FlowContainer(const frameflow::FlowData &data);

    void InitializeLayout(LayoutSystem *system) override;
};

class CenterContainer : public Control {
public:
    explicit CenterContainer() = default;

    void InitializeLayout(LayoutSystem *system) override;
};

class MarginContainer : public Control {
public:
    frameflow::MarginData margin_data;

    explicit MarginContainer(const frameflow::MarginData &data);

    void InitializeLayout(LayoutSystem *system) override;
};

class HBFont;

class Label : public Control {
public:
    std::string text;
    HBFont *font{nullptr};
    Color color{BLACK};

    explicit Label() = default;

    void Draw() override;

    void Update(float delta_time) override;

private:
    float ascent{};
};

class LineInput : public Control {
public:
    std::string text;
    HBFont *font{nullptr};
    Color color{BLACK};
    bool has_focus{false}; // focusGroup?

    explicit LineInput() = default;

    void Draw() override;

    void Update(float delta_time) override;;
};
