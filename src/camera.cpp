#include "camera.h"

Camera::Camera() : 
    pos(0,0,-4),
    ri(1,0,0), up(0,1,0), fo(0,0,1),
    fovY(glm::radians(90.0f)), 
    nearClip(0.01f), farClip(500.0f)
    {}

vec3 Camera::Raycast(vec2 uv) const {
    const float tempRatio = 640.0f / 480.0f; // TODO: make like global variable or smth
    vec2 p = uv*vec2(2.0f)-vec2(1.0f);
    vec3 rdCam = normalize(vec3(p*tan(fovY*0.5f)*vec2(tempRatio, 1.0f), 1.0f));
    return mat3(ri, up, fo) * rdCam;
}

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