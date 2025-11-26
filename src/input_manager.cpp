#include "input_manager.h"
#include <iostream>

void InputManager::AddOnMouseMoveListener(const std::function<void(vec2, vec2)> &f) {
    onMouseMoveListeners.push_back(f);
}
void InputManager::AddOnMouseClickListener(const std::function<void(vec2)> &f) {
    onMouseClickListeners.push_back(f);
}
void InputManager::AddOnMouseReleaseListener(const std::function<void(vec2)> &f) {
    onMouseReleaseListeners.push_back(f);
}
void InputManager::AddOnMouseScrollListener(const std::function<void(float)> &f) {
    onMouseScrollListeners.push_back(f);
}

void InputManager::OnMouseMove(vec2 newPos) {
    for(auto &f : onMouseMoveListeners) {
        f(newPos, newPos - mousePosition);
    }
    mousePosition = newPos;
}

void InputManager::OnMouseClick(bool leftMouse, bool pressDown) {
    if(leftMouse) {
        lMouseDown = pressDown;
        if(pressDown) {
            for(auto &f : onMouseClickListeners) {
                f(mousePosition);
            }
        } else {
            for(auto &f : onMouseReleaseListeners) {
                f(mousePosition);
            }
        }
    } else {
        rMouseDown = pressDown;
    }
}

void InputManager::OnMouseScroll(float scroll) {
    for(auto &f : onMouseScrollListeners) {
        f(scroll);
    }
}