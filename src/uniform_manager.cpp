#include "marching_cubes_manager.h"

void MarchingCubes::UniformManager::Initialize() {
    assert(manager->guiToParams.initialized);

    // Initialize Buffers
    parameterBuffer = createBuffer(manager->device, sizeof(Parameters), 
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    brushParameterBuffer = createBuffer(manager->device, sizeof(BrushParameters),
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    cameraTimeUniformBuffer = createBuffer(manager->device, sizeof(CameraTimeUniform),
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    raycastInputUniformBuffer = createBuffer(manager->device, sizeof(RaycastInputUniform),
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    
    // Send Initial Data
    UpdateParameters();
    UpdateBrushParameters();
    UpdateCameraTimeParameters();

    std::cout << "Initalized Uniform Manager" << std::endl;
    initialized = true;
}

void MarchingCubes::UniformManager::UpdateParameters() {
    manager->queue.writeBuffer(parameterBuffer.buffer, 0, &manager->guiToParams.parameters, sizeof(Parameters));
}
void MarchingCubes::UniformManager::UpdateBrushParameters() {
    manager->queue.writeBuffer(brushParameterBuffer.buffer, 0, &manager->guiToParams.brushParameters, sizeof(BrushParameters));
}
void MarchingCubes::UniformManager::UpdateCameraTimeParameters() {
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, 0, &manager->guiToParams.cameraTimeParameters, sizeof(CameraTimeUniform));
}
// void MarchingCubes::UniformManager::UpdateBrushMult(float);
// void MarchingCubes::UniformManager::UpdateBrushSize(float);
// void MarchingCubes::UniformManager::UpdateBrushType(uint32);

void MarchingCubes::UniformManager::UpdateViewMatrix() {
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, viewMatrix), &manager->guiToParams.cameraTimeParameters.viewMatrix, sizeof(mat4x4));
}
void MarchingCubes::UniformManager::UpdateProjectionMatrix() {
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, projectionMatrix), &manager->guiToParams.cameraTimeParameters.projectionMatrix, sizeof(mat4x4));
}
void MarchingCubes::UniformManager::UpdateModelMatrix() {
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, modelMatrix), &manager->guiToParams.cameraTimeParameters.modelMatrix, sizeof(mat4x4));
}
void MarchingCubes::UniformManager::UpdateTime() {
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, time), &manager->guiToParams.cameraTimeParameters.time, sizeof(float));
}

void MarchingCubes::UniformManager::SetRaycastInput(vec3 origin, vec3 direction) {
    auto raycastInput = RaycastInputUniform(origin, direction);
    manager->queue.writeBuffer(raycastInputUniformBuffer.buffer, 0, &raycastInput, sizeof(RaycastInputUniform));
}

void MarchingCubes::UniformManager::Destroy() {
    parameterBuffer.buffer.release();
    brushParameterBuffer.buffer.release();
    cameraTimeUniformBuffer.buffer.release();
}