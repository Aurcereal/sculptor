#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace glm;

class Camera {
    private:

    vec3 pos;
    vec3 ri, up, fo;
    float fovY;
    float nearClip, farClip;
    
    public:

    vec3 Raycast(vec2 uv) const;
    mat4 GetViewMatrix() const;
    mat4 GetProjectionMatrix() const;
    Camera();
};