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
    assert(manager->meshGenerator.initialized && manager->uniformManager.initialized);

    // Initialize Depth Texture
    depthTextureHolder.Initialize(manager->device);

    // Initialize Shader
    vector<SP::Parameter> shaderParams = {
      SP::Parameter(SP::UUniform{&manager->uniformManager.cameraTimeUniformBuffer}),
      SP::Parameter(SP::UTexture{&manager->fieldEditor.fieldColorTexture, true, false}),
      SP::Parameter(SP::USampler{&manager->fieldEditor.fieldColorTexture}),
      SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer})
    };
    drawShader.Initialize(manager->device, shaderParams, surfaceFormat, depthTextureHolder, "./drawing/blinn_phong.wgsl");

    // Initialize Indirect Args
    std::array<uint32_t, 5> indirectArgs = {
        0,     // Index Count TODO: make initially 0 and we take count buffer to index buffer
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
    initialized = true;

    waitForQueueCompletion(manager->device, manager->queue);
}

void MarchingCubes::Drawer::UpdateIndexCount() {
    fillIndexCountShader.DispatchSync(manager->device, manager->queue, uvec3(1));
}

void MarchingCubes::Drawer::Destroy() {
    indirectDrawArgs.buffer.release();
    fillIndexCountShader.Destroy();
    depthTextureHolder.Destroy();
    drawShader.Destroy();
}