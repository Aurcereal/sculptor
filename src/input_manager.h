#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>

using namespace glm;
using namespace std;

class InputManager {
public:
    void OnMouseMove(vec2 newPos);
    void OnMouseClick();

    void AddOnMouseMoveListener(const std::function<void(vec2)>&);
    void AddOnMouseClickListener(const std::function<void(vec2)>&);

private:
    vec2 mousePosition;

    vector<std::function<void(vec2)>> onMouseMoveListeners;
    vector<std::function<void(vec2)>> onMouseClickListeners;
};