#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>

using namespace glm;
using namespace std;

class InputManager {
public:
    void OnMouseMove(vec2 newPos);
    void OnMouseClick(bool leftMouse, bool pressDown);
    void OnMouseScroll(float scroll);

    void AddOnMouseMoveListener(const std::function<void(vec2, vec2)>&);
    void AddOnMouseClickListener(const std::function<void(vec2)>&);
    void AddOnMouseReleaseListener(const std::function<void(vec2)>&);
    void AddOnMouseScrollListener(const std::function<void(float)>&);

    vec2 mousePosition;
    bool lMouseDown = false;
    bool rMouseDown = false;

private:
    vector<std::function<void(vec2, vec2)>> onMouseMoveListeners;
    vector<std::function<void(vec2)>> onMouseClickListeners;
    vector<std::function<void(vec2)>> onMouseReleaseListeners;
    vector<std::function<void(float)>> onMouseScrollListeners;
};