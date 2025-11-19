#pragma once

#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <utility>
#include <glm/glm.hpp>

using namespace wgpu;

class DepthTexture {
    public:

    void Initialize(Device&);

    DepthStencilState depthStencilState;
    Texture depthTexture = nullptr;
    TextureView depthTextureView = nullptr;
    
};