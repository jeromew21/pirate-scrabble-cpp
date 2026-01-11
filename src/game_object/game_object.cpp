#include "game_object.h"

GameObject::~GameObject() = default;

void GameObject::UpdateRec(float delta_time) {
    Update(delta_time);
    for (auto& child : children) {
        child->UpdateRec(delta_time);
    }
}

void GameObject::DrawRec() {
    Draw();
    for (auto& child : children) {
        child->DrawRec();
    }
}

void GameObject::AddChild(GameObject* child) {
    child->parent = this;
    children.push_back(child);
    AddChildHook(child);
}

void GameObject::Draw() {}

void GameObject::Update(float delta_time) {}

void GameObject::AddChildHook(GameObject *child) {}
