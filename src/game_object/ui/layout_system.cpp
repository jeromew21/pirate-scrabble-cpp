#include "layout_system.h"

#include "frameflow/layout.hpp"
#include "raylib.h"

#include "control.h"

LayoutSystem::LayoutSystem() : system(std::make_unique<frameflow::System>()) {
    root_node_id = frameflow::add_generic(system.get(), frameflow::NullNode);
}

void LayoutSystem::Update(float delta_time) {
    if (fill_screen) {
        const int win_width = GetScreenWidth();
        const int win_height = GetScreenHeight();
        frameflow::Node &node = *frameflow::get_node(system.get(), root_node_id);
        node.bounds.size = {static_cast<float>(win_width), static_cast<float>(win_height)};
        node.minimum_size = {static_cast<float>(win_width), static_cast<float>(win_height)};
    }
    // could we have other resizing methods?
    frameflow::compute_layout(system.get(), root_node_id);
}

void LayoutSystem::AddChildHook(GameObject *child) {
    if (const auto child_control = dynamic_cast<Control *>(child); child_control != nullptr) {
        child_control->layout_system_ = this;
        child_control->InitializeLayout(this);
    }
}
