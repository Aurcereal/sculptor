#pragma once

#include <webgpu/webgpu.hpp>

// #include <GLFW/glfw3.h>
// #include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <utility>
#include <glm/glm.hpp>
#include <vector>
#include "texture_holder.h"

using namespace wgpu;
using namespace std;

class ComputeShader {
    public:

    inline ComputeShader() {}
    void Initialize(Device &device, TextureHolder&, TextureHolder&);
    void Dispatch(Queue &queue, CommandEncoder &encoder, uvec3 size);
    void Destroy();

    private:

    void InitBindGroupLayout(Device &device);
    void InitBindGroups(Device &device, TextureHolder&, TextureHolder&);
    PipelineLayout pipelineLayout = nullptr;
    ComputePipeline computePipeline = nullptr;

    BindGroupLayout bindGroupLayout = nullptr;
    BindGroup bindGroup = nullptr;
};