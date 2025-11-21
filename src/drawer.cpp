#include "marching_cubes_manager.h"
#include <array>

struct DrawUniforms {
    mat4x4 projectionMatrix;
    mat4x4 viewMatrix;
    mat4x4 modelMatrix;
    float time;
    float _pad[3];
};

namespace SP = ShaderParameter;

void MarchingCubes::Drawer::Initialize(TextureFormat surfaceFormat) {
    // Initialize Uniforms
    uniformBuffer = createBuffer(manager->device, sizeof(DrawUniforms), 
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    
    DrawUniforms drawUniforms = { // Lucky that padding is at the end
        manager->camera.GetProjectionMatrix(),
        manager->camera.GetViewMatrix(),
        mat4(1.0f),
        0.0f
    };
    manager->queue.writeBuffer(uniformBuffer.buffer, 0, &drawUniforms, sizeof(DrawUniforms));

    // Initialize Depth Texture
    depthTextureHolder.Initialize(manager->device);

    // Initialize Shader
    vector<SP::Parameter> shaderParams = {
      SP::Parameter(SP::UUniform{&uniformBuffer})
    };
    drawShader.Initialize(manager->device, shaderParams, surfaceFormat, depthTextureHolder, "./drawing/flat_lambert.wgsl");

    // Initialize Indirect Args
    std::array<uint32_t, 5> indirectArgs = {
        0,//262144, // Index Count TODO: make initially 0 and we take count buffer to index buffer
        1,     // Instance Count
        0,     // First Index
        0,     // Base Vertex
        0      // First Instance
    };
    indirectDrawArgs = createBuffer(manager->device, sizeof(uint32_t) * indirectArgs.size(),
        BufferUsage::CopyDst | BufferUsage::Storage | BufferUsage::Indirect, false);
    manager->queue.writeBuffer(indirectDrawArgs.buffer, 0, indirectArgs.data(), indirectDrawArgs.size);

    // Initialize Indirect Args Filler
    vector<SP::Parameter> argsFillerShaderParams = {
        SP::Parameter(SP::UBuffer{&manager->meshGenerator.countBuffer, false}),
        SP::Parameter(SP::UBuffer{&indirectDrawArgs, true})
    };
    fillIndexCountShader.Initialize(manager->device, argsFillerShaderParams, "./drawing/fill_index_count.wgsl");

    std::cout << "Initialized Drawer" << std::endl;
}

void MarchingCubes::Drawer::UpdateUniforms() {
    float t = static_cast<float>(glfwGetTime());
    manager->queue.writeBuffer(uniformBuffer.buffer, offsetof(DrawUniforms, time), &t, sizeof(float));
    mat4x4 viewMat = manager->camera.GetViewMatrix();
    mat4x4 modelMat = glm::rotate(mat4(1.0f), t, vec3(0.0f, 1.0f, 0.0f));
    manager->queue.writeBuffer(uniformBuffer.buffer, offsetof(DrawUniforms, modelMatrix), &modelMat, sizeof(mat4x4));
    manager->queue.writeBuffer(uniformBuffer.buffer, offsetof(DrawUniforms, viewMatrix), &viewMat, sizeof(mat4x4));
}

void MarchingCubes::Drawer::UpdateIndexCount() {
    fillIndexCountShader.DispatchSync(manager->device, manager->queue, uvec3(1));
}

void MarchingCubes::Drawer::Destroy() {
    indirectDrawArgs.buffer.release();
    fillIndexCountShader.Destroy();
    uniformBuffer.buffer.release();
    depthTextureHolder.Destroy();
    drawShader.Destroy();
}