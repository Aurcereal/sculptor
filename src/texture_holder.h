#pragma once

#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <glm/glm.hpp>

using namespace wgpu;
using namespace glm;

class TextureHolder {
    public:
    TextureHolder();
    Texture texture = nullptr;
    TextureDescriptor textureDesc;
    TextureView textureView = nullptr;
    Sampler sampler = nullptr;
    uvec3 textureSize;

    void Initialize(Device &device, uvec3 size, bool is3D, bool shaderWrite);
    // 8-BIT NEEDS TO CHANGE if format changes
    void WriteToTexture(Queue&, const std::vector<uint8_t> &pixels);
    void Destroy();
};