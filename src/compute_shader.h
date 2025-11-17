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
#include "shader_parameter.h"
#include "webgpu_utils.h"

using namespace wgpu;
using namespace std;

class ComputeShader {
    public:

    inline ComputeShader() {}
    void Initialize(Device &device, const vector<ShaderParameter::Parameter>&);//TextureHolder&, TextureHolder&);
    void Dispatch(Device &device, Queue &queue, uvec3 size);
    void Destroy();

    private:

    void InitBindGroupLayout(Device &device, const vector<ShaderParameter::Parameter>&);
    void InitBindGroups(Device &device, const vector<ShaderParameter::Parameter>&);//TextureHolder&, TextureHolder&);
    PipelineLayout pipelineLayout = nullptr;
    ComputePipeline computePipeline = nullptr;

    BindGroupLayout bindGroupLayout = nullptr;
    BindGroup bindGroup = nullptr;
};