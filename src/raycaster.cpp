#include "marching_cubes_manager.h"
#include <iostream>
#include <array>

namespace SP = ShaderParameter;

void MarchingCubes::Raycaster::Initialize() {
    intersectionBuffer = createBuffer(manager->device, sizeof(vec4)*5,
        BufferUsage::Storage | BufferUsage::CopyDst | BufferUsage::CopySrc, false);
    ResetIntersectionBuffer();

    initialized = true;
}

void MarchingCubes::Raycaster::ResetIntersectionBuffer() {
    array<vec4, 5> intersectInfo = {vec4(0), vec4(0), vec4(0), vec4(0), vec4(0)};
    manager->queue.writeBuffer(intersectionBuffer.buffer, 0, intersectInfo.data(), 5*sizeof(vec4));
    waitForQueueCompletion(manager->device, manager->queue);
}

void MarchingCubes::Raycaster::InitializeWithDependencies() {
    assert(manager->fieldEditor.initialized && manager->uniformManager.initialized);

    const TextureHolder& fieldTexture = manager->intersectionTextureGeneratedOnlyOnClick ? 
            manager->fieldEditor.fieldIntersectionTexture : manager->fieldEditor.fieldTexture;

    vector<SP::Parameter> params = {
        SP::Parameter(SP::UTexture{&fieldTexture, true, false}),
        SP::Parameter(SP::USampler{&fieldTexture}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.raycastInputUniformBuffer}),
        SP::Parameter(SP::UBuffer{&intersectionBuffer, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer}),
    };
    computeIntersection.Initialize(manager->device, params, "./raycasting/ray_field_intersect.wgsl");
}

void MarchingCubes::Raycaster::RayFieldIntersect(vec3 origin, vec3 direction, bool writeNormal) {
    manager->uniformManager.SetRaycastInput(origin, direction, writeNormal);
    computeIntersection.DispatchSync(manager->device, manager->queue, uvec3(1));
}

void MarchingCubes::Raycaster::RayFieldIntersect(vec3 origin, vec3 direction, vec3 *outPosition, mat3x3 *outFrame, bool writeNormal) {
    manager->uniformManager.SetRaycastInput(origin, direction, writeNormal);
    computeIntersection.DispatchSync(manager->device, manager->queue, uvec3(1));

    vec4* intersectionData = (vec4*) BufferHelper::MapBufferToCPU(manager->device, manager->queue, intersectionBuffer);
    *outPosition = intersectionData[0];
    *outFrame = mat3x3(
                    vec3(intersectionData[2]),
                    vec3(intersectionData[3]),
                    vec3(intersectionData[1])
                );
    BufferHelper::Unmap();
}

void MarchingCubes::Raycaster::SendOffset(vec2 offset) {
    vec4 offsetSend = vec4(offset.x, offset.y, 0.0f, 0.0f);
    manager->queue.writeBuffer(intersectionBuffer.buffer, sizeof(vec4) * 4, &offset, sizeof(vec4));
    waitForQueueCompletion(manager->device, manager->queue);
}