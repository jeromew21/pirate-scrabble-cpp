#include "control.h"

#include <optional>

#include <frameflow/layout.hpp>
#include <raylib.h>

#include "layout_system.h"
#include "text/texthb.h"

// Try to convert node to frameflow node.
// If parent is a LayoutSystem, consider that NullNode.
// Otherwise, return nullopt.
static std::optional<frameflow::NodeId> as_frameflow_node(GameObject *node) {
    if (node == nullptr) return std::nullopt;
    if (auto *sys = dynamic_cast<LayoutSystem *>(node)) {
        return sys->root_node_id;
    }
    if (auto *control = dynamic_cast<Control *>(node); control != nullptr) {
        return control->node_id_;
    }
    return std::nullopt;
}

static Color color_for(frameflow::NodeType type) {
    using namespace frameflow;
    switch (type) {
        case NodeType::Center: return BLUE;
        case NodeType::Box: return GREEN;
        case NodeType::Flow: return ORANGE;
        case NodeType::Generic: return RAYWHITE;
        default: return MAGENTA;
    }
}

static const char *node_type_name(frameflow::NodeType type) {
    using namespace frameflow;
    switch (type) {
        case NodeType::Center: return "Center";
        case NodeType::Box: return "Box";
        case NodeType::Flow: return "Flow";
        case NodeType::Generic: return "Generic";
        default: return "Unknown";
    }
}


/**
 * This must be called after AddChild
 * Unsafe pointer! Could die after move
 */
frameflow::Node *Control::GetNode() const {
    return frameflow::get_node(layout_system_->system.get(), node_id_);
}

void Control::Draw() {
    using namespace frameflow;
    const auto node = *GetNode();

    const Rectangle r{
        node.bounds.origin.x,
        node.bounds.origin.y,
        node.bounds.size.x,
        node.bounds.size.y
    };

    //if (node.parent != NullNode) {
    Color c = color_for(node.type);
    DrawRectangleLinesEx(r, 1.0f, c);

    // Draw node type text in top-left corner
    const char *type_str = node_type_name(node.type);
    int font_size = 12;
    DrawText(type_str,
             static_cast<int>(node.bounds.origin.x) + 2,
             static_cast<int>(node.bounds.origin.y) + 2,
             font_size,
             c);
    // }
}

void Control::AddChildHook(GameObject *child) {
    if (auto child_control = dynamic_cast<Control *>(child); child_control != nullptr) {
        child_control->layout_system_ = layout_system_;
        child_control->InitializeLayout(layout_system_);
    }
}

BoxContainer::BoxContainer(const frameflow::BoxData &data) : box_data(data) {
}

void Control::InitializeLayout(LayoutSystem *system) {
    if (const auto parent_node = as_frameflow_node(parent); parent_node.has_value()) {
        node_id_ = frameflow::add_generic(system->system.get(), parent_node.value());
    }
}

void BoxContainer::InitializeLayout(LayoutSystem *system) {
    if (const auto parent_node = as_frameflow_node(parent); parent_node.has_value()) {
        node_id_ = frameflow::add_box(system->system.get(),
                           parent_node.value(),
                           box_data);
    }
}

void CenterContainer::InitializeLayout(LayoutSystem *system) {
    if (const auto parent_node = as_frameflow_node(parent); parent_node.has_value()) {
        node_id_ = frameflow::add_center(system->system.get(),
                              parent_node.value());
    }
}

void Label::Draw() {
    Control::Draw();
    //fallback color and font!
    auto measure = MeasureTextHB(*font, text);
    DrawTextHB(*font,
        text,
        GetNode()->bounds.origin.x,
        GetNode()->bounds.origin.y + measure.ascent,
        *color);
    GetNode()->minimum_size = {measure.width, measure.height};
}
