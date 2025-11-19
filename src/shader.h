#pragma once

#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <utility>
#include <glm/glm.hpp>

#include "shader_parameter.h"
#include "depth_texture.h"

using namespace std;

class Shader {
    public:
    
    void Initialize(Device&, const vector<ShaderParameter::Parameter>&, TextureFormat, DepthTexture&, std::string path);
    void Destroy();
    inline Shader() {}

    RenderPipeline pipeline = nullptr;
    BindGroup bindGroup = nullptr;

    private:

    void InitBindGroupLayout(Device &device, const vector<ShaderParameter::Parameter>&);
    void InitBindGroups(Device &device, const vector<ShaderParameter::Parameter>&);

    VertexBufferLayout InitVertexLayout(vector<VertexAttribute>&);

    PipelineLayout pipelineLayout = nullptr;
    
    TextureView depthTextureView = nullptr;

    BindGroupLayout bindGroupLayout = nullptr;
    
    
};