#include "camera.h"

Camera::Camera() : 
    pos(0,0,-4),
    ri(1,0,0), up(0,1,0), fo(0,0,1),
    fovY(glm::radians(90.0f)), 
    nearClip(0.01f), farClip(500.0f)
    {}

mat4x4 Camera::GetViewMatrix() const {
    mat4 rot = transpose(mat4(
        vec4(ri, 0.0f),
        vec4(up, 0.0f),
        vec4(-fo, 0.0f),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    ));
    mat4 trans = glm::translate(mat4(1.0f), -pos);
    return rot * trans;
}

mat4x4 Camera::GetProjectionMatrix() const {
    const float tempRatio = 640.0f / 480.0f;
    return glm::perspective(fovY, tempRatio, nearClip, farClip);
}