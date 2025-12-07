#include "marching_cubes_manager.h"
#include <iostream>
#include <array>

namespace SP = ShaderParameter;

void MarchingCubes::Raycaster::Initialize() {
    intersectionBuffer = createBuffer(manager->device, sizeof(vec4)*2,
        BufferUsage::Storage | BufferUsage::CopyDst | BufferUsage::CopySrc, false);
    ResetIntersectionBuffer();

    initialized = true;
}

void MarchingCubes::Raycaster::ResetIntersectionBuffer() {
    array<vec4, 2> intersectInfo = {vec4(0), vec4(0)};
    manager->queue.writeBuffer(intersectionBuffer.buffer, 0, intersectInfo.data(), 2*sizeof(vec4));
}

void MarchingCubes::Raycaster::InitializeWithDependencies() {
    assert(manager->fieldEditor.initialized && manager->uniformManager.initialized);

    vector<SP::Parameter> params = {
        SP::Parameter(SP::UTexture{&manager->fieldEditor.fieldTexture, true, false}),
        SP::Parameter(SP::USampler{&manager->fieldEditor.fieldTexture}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.raycastInputUniformBuffer}),
        SP::Parameter(SP::UBuffer{&intersectionBuffer, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer}),
    };
    computeIntersection.Initialize(manager->device, params, "./raycasting/ray_field_intersect.wgsl");
}

void MarchingCubes::Raycaster::RayFieldIntersect(vec3 origin, vec3 direction, bool writeNormal) {
    manager->uniformManager.SetRaycastInput(origin, direction, writeNormal);
    computeIntersection.DispatchSync(manager->device, manager->queue, uvec3(1));
    vec4* intersectionData = (vec4*) BufferHelper::MapBufferToCPU(manager->device, manager->queue, intersectionBuffer);
    std::cout << "x: " << intersectionData[0].x << "y: " << intersectionData[0].y << "z: " << intersectionData[0].z << std::endl;
    BufferHelper::Unmap();
}