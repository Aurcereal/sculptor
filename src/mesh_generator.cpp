#include "marching_cubes_manager.h"

namespace SP = ShaderParameter;

void MarchingCubes::MeshGenerator::Initialize() {
    assert(manager->guiToParams.initialized);

    // Initialize Buffers
    vertexBuffer = createBuffer(manager->device, sizeof(float)*6*maxVertexCount, // Vertex Size = 6 (Make Parameter) 
        BufferUsage::Vertex | BufferUsage::Storage, false);
    indexBuffer = createBuffer(manager->device, sizeof(uint32_t)*3 * maxTriangleCount, 
        BufferUsage::Index | BufferUsage::Storage, false);
    countBuffer = createBuffer(manager->device, sizeof(uint32_t) * 2, 
        BufferUsage::CopyDst | BufferUsage::Storage, false);

    // Initialize Shader
    vector<SP::Parameter> shaderParams = {
        SP::Parameter(SP::UBuffer{&vertexBuffer, true}),
        SP::Parameter(SP::UBuffer{&indexBuffer, true}),
        SP::Parameter(SP::UBuffer{&countBuffer, true}),
        SP::Parameter(SP::UTexture{&manager->fieldEditor.fieldTexture, true, false}),
        SP::Parameter(SP::USampler{&manager->fieldEditor.fieldTexture}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer})
    };
    meshGenerationShader.Initialize(manager->device, shaderParams, "/mesh_generator/generate_mesh.wgsl");
    
    std::cout << "Initialized Mesh Generator" << std::endl;
    initialized = true;
}

void MarchingCubes::MeshGenerator::ResetCountBuffer() {
    vector<uint32_t> counts = {0,0};
    manager->queue.writeBuffer(countBuffer.buffer, 0, counts.data(), countBuffer.size);
}

void MarchingCubes::MeshGenerator::GenerateMesh() {
    ResetCountBuffer();
    meshGenerationShader.DispatchSync(manager->device, manager->queue, uvec3(manager->guiToParams.parameters.marchingCubesResolution));
}

void MarchingCubes::MeshGenerator::Destroy() {
    vertexBuffer.buffer.release();
    indexBuffer.buffer.release();
    countBuffer.buffer.release();

    meshGenerationShader.Destroy();
}