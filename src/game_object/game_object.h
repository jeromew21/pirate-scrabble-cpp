#pragma once

#pragma once

#include <vector>

class GameObject {
public:
    virtual ~GameObject();

    void UpdateRec(float delta_time);

    void DrawRec();

    void AddChild(GameObject *child);

protected:
    virtual void Draw();

    virtual void Update(float delta_time);

    virtual void AddChildHook(GameObject *child);;

    GameObject* parent{nullptr};

    std::vector<GameObject*> children;
};
