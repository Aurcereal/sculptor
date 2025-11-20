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
    TextureFormat format = TextureFormat::Undefined;
    Sampler sampler = nullptr;
    uvec3 textureSize;

    void Initialize(Device&, uvec3 size, TextureFormat, bool is3D, bool shaderWrite);
    //void WriteToTexture(Queue&, const std::vector<uint8_t> &pixels); Only works for RGBA8 rn
    void Destroy();
};