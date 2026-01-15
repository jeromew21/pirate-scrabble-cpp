#include "game_object.h"

#include <algorithm>

GameObject::~GameObject() = default;

void GameObject::UpdateRec(const float delta_time) {
    Update(delta_time);
    for (const auto &child: children) {
        child->UpdateRec(delta_time);
    }
}

void GameObject::DrawRec() {
    if (!visible_in_tree) return;
    Draw();
    for (const auto &child: children) {
        child->DrawRec();
    }
}

GameObject *GameObject::AddChild(GameObject *child) {
    child->parent = this;
    children.push_back(child);
    AddChildHook(child);
    return child;
}

void GameObject::Show() {
    visible_self = true;
    UpdateVisibilityFromParent();
}

void GameObject::Hide() {
    visible_self = false;
    UpdateVisibilityFromParent();
}

bool GameObject::IsVisible() const {
    return visible_in_tree;
}

void GameObject::Delete() {
    if (parent) {
        auto& siblings = parent->children;
        std::erase(siblings, this);
    }
    DeleteHook();
    DeleteRec();
}

void GameObject::DeleteRec() {
    for (auto *child: children) {
        if (child) child->DeleteRec();
    }
    delete this;
}

std::vector<GameObject *> GameObject::GetChildren() {
    return children;
}

// Default impl, does nothing
void GameObject::Draw() {
}

// Default impl, does nothing
void GameObject::Update(float) {
}

void GameObject::AddChildHook(GameObject *) {
}

void GameObject::DeleteHook() {
}

void GameObject::UpdateVisibilityFromParent() {
    const bool parent_visible = parent ? parent->visible_in_tree : true;
    const bool new_visible = visible_self && parent_visible;

    if (new_visible == visible_in_tree)
        return;

    visible_in_tree = new_visible;

    for (auto *c: children)
        c->UpdateVisibilityFromParent();
}
