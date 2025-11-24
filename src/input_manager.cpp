#include "input_manager.h"

void InputManager::AddOnMouseMoveListener(const std::function<void(vec2)> &f) {
    onMouseMoveListeners.push_back(f);
}
void InputManager::AddOnMouseClickListener(const std::function<void(vec2)> &f) {
    onMouseClickListeners.push_back(f);
}
void InputManager::AddOnMouseReleaseListener(const std::function<void(vec2)> &f) {
    onMouseReleaseListeners.push_back(f);
}

void InputManager::OnMouseMove(vec2 newPos) {
    for(auto &f : onMouseMoveListeners) {
        f(newPos - mousePosition);
    }
    mousePosition = newPos;
}

void InputManager::OnMouseClick(bool leftMouse, bool pressDown) {
    if(!leftMouse) return; // For now
    if(pressDown) {
        for(auto &f : onMouseClickListeners) {
            f(mousePosition);
        }
    } else {
        for(auto &f : onMouseReleaseListeners) {
            f(mousePosition);
        }
    }
}