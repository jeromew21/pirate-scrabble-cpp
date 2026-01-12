#pragma once

#pragma once

#include <vector>

class GameObject {
public:
    virtual ~GameObject();

    void UpdateRec(float delta_time);

    void DrawRec();

    void AddChild(GameObject *child);

    void Show();

    void Hide();

    [[nodiscard]] bool IsVisible() const;

protected:
    virtual void Draw();

    virtual void Update(float delta_time);

    virtual void AddChildHook(GameObject *child);

    GameObject *parent{nullptr};

    std::vector<GameObject *> children;

private:
    void UpdateVisibilityFromParent();

    bool visible_self{true};

    bool visible_in_tree{true};
};
