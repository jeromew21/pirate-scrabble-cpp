#pragma once

#pragma once

#include <vector>
#include <memory>

class GameObject {
public:
    virtual ~GameObject();

    virtual void UpdateRec(float delta_time);

    void DrawRec();

    void AddChild(GameObject *child);

protected:
    virtual void Draw();

    virtual void Update(float delta_time);

private:
    GameObject* parent{nullptr};
    std::vector<GameObject*> children;
};
