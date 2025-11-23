#include "marching_cubes_manager.h"

void MarchingCubes::UniformManager::Initialize(uint32 texRes, uint32 marchingCubesRes, const Camera& cam) {
    // Initialize structs
    parameters = {texRes, marchingCubesRes};
    brushParameters = {mat4x4(1.0f), 0, 1.0f, 1.0f};
    cameraTimeParameters = {cam.GetProjectionMatrix(), cam.GetViewMatrix(), mat4(1.0f), 0.0f};

    // Initialize Buffers
    parameterBuffer = createBuffer(manager->device, sizeof(Parameters), 
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    brushParameterBuffer = createBuffer(manager->device, sizeof(BrushParameters),
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    cameraTimeUniformBuffer = createBuffer(manager->device, sizeof(CameraTimeUniform),
        BufferUsage::CopyDst | BufferUsage::Uniform, false);
    
    // Send Initial Data
    manager->queue.writeBuffer(parameterBuffer.buffer, 0, &parameters, sizeof(Parameters));
    manager->queue.writeBuffer(brushParameterBuffer.buffer, 0, &brushParameters, sizeof(BrushParameters));
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, 0, &cameraTimeParameters, sizeof(CameraTimeUniform));

    std::cout << "Initalized Uniform Manager" << std::endl;
}

void MarchingCubes::UniformManager::UpdateResolutions(const uint32 *newTexResolution, const uint32 *newMarchingCubesResolution) {
    if(newTexResolution) {
        parameters.textureResolution = *newTexResolution;
        manager->queue.writeBuffer(parameterBuffer.buffer, offsetof(Parameters, textureResolution), &parameters.textureResolution, sizeof(uint32_t));
    }
    if(newMarchingCubesResolution) {
        parameters.marchingCubesResolution = *newMarchingCubesResolution;
        manager->queue.writeBuffer(parameterBuffer.buffer, offsetof(Parameters, marchingCubesResolution), &parameters.marchingCubesResolution, sizeof(uint32_t));
    }
}

// void MarchingCubes::UniformManager::UpdateBrushTransform(const mat4x4&);
// void MarchingCubes::UniformManager::UpdateBrushMult(float);
// void MarchingCubes::UniformManager::UpdateBrushSize(float);
// void MarchingCubes::UniformManager::UpdateBrushType(uint32);

void MarchingCubes::UniformManager::UpdateViewMatrix(const Camera& cam) {
    cameraTimeParameters.viewMatrix = cam.GetViewMatrix();
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, viewMatrix), &cameraTimeParameters.viewMatrix, sizeof(mat4x4));
}
void MarchingCubes::UniformManager::UpdateProjectionMatrix(const Camera& cam) {
    cameraTimeParameters.projectionMatrix = cam.GetProjectionMatrix();
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, projectionMatrix), &cameraTimeParameters.projectionMatrix, sizeof(mat4x4));
}
void MarchingCubes::UniformManager::UpdateModelMatrix(const mat4x4& mat) {
    cameraTimeParameters.modelMatrix = mat;
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, modelMatrix), &cameraTimeParameters.modelMatrix, sizeof(mat4x4));
}
void MarchingCubes::UniformManager::UpdateTime() {
    cameraTimeParameters.time = static_cast<float>(glfwGetTime());
    manager->queue.writeBuffer(cameraTimeUniformBuffer.buffer, offsetof(CameraTimeUniform, time), &cameraTimeParameters.time, sizeof(float));
}

void MarchingCubes::UniformManager::Destroy() {
    parameterBuffer.buffer.release();
    brushParameterBuffer.buffer.release();
    cameraTimeUniformBuffer.buffer.release();
}