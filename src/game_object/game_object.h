#pragma once

#include <vector>

class GameObject {
public:
    virtual ~GameObject();

    void UpdateRec(float delta_time);

    void DrawRec();

    GameObject* AddChild(GameObject *child);

    void Show();

    void Hide();

    [[nodiscard]] bool IsVisible() const;

    void Delete();

    std::vector<GameObject *> GetChildren();

protected:
    virtual void Draw();

    virtual void Update(float delta_time);

    virtual void AddChildHook(GameObject *child);

    virtual void DeleteHook();

    GameObject *parent{nullptr};

    std::vector<GameObject *> children;

private:
    void UpdateVisibilityFromParent();

    bool visible_self{true};

    bool visible_in_tree{true};
};
