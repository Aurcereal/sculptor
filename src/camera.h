#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "input_manager.h"

using namespace glm;

class Camera {
private:

    vec3 ri, up, fo;
    float fovY;
    float nearClip, farClip;
    vec3 target;
    float distFromTarget;

    void RotateAlongGlobalY(float);
    void RotateAlongLocalX(float);
    void MoveAlongLocalXY(vec2);

    void OnMouseMove(vec2 newPos, vec2 delta);
    InputManager *inputManager;
    
public:
    vec3 pos;

    vec3 Raycast(vec2 uv) const;
    mat4 GetViewMatrix() const;
    mat4 GetProjectionMatrix() const;
    Camera(InputManager*);
};