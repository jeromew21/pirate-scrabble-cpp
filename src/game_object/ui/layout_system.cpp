#include "layout_system.h"

#include <frameflow/layout.hpp>
#include <raylib.h>

#include "control.h"

LayoutSystem::LayoutSystem(): system_(std::make_unique<frameflow::System>()) {

}

void LayoutSystem::Update(float delta_time) {
    int win_width = 0;
    int win_height = 0;
    if (fill_screen) {
        win_width = GetScreenWidth();
        win_height = GetScreenHeight();
    }
    for (auto *child : children) {
        if (const auto child_control = dynamic_cast<Control *>(child); child_control != nullptr) {
            if (fill_screen) {
                frameflow::Node *node = child_control->GetNode();
                node->bounds.size = {static_cast<float>(win_width), static_cast<float>(win_height)};
                node->minimum_size = {static_cast<float>(win_width), static_cast<float>(win_height)};
            }
            frameflow::compute_layout(system_.get(), child_control->node_id_);
        }
    }
}

void LayoutSystem::AddChildHook(GameObject *child) {
    if (const auto child_control = dynamic_cast<Control *>(child); child_control != nullptr) {
        child_control->layout_system_ = this;
        child_control->InitializeLayout(this);
    }
}
