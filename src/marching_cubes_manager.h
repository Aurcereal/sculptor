#pragma once

// We shouldn't have to communicate with any raw webgpu here ideally...
#include "texture_holder.h"
#include "compute_shader.h"

#include <glm/glm.hpp>

using namespace glm;

class MarchingCubesManager {
    public:
    void Initialize();
    void Run();

    private:

    uint textureResolution;
    ComputeShader meshGeneratorShader;
};