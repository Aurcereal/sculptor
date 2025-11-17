#pragma once

// We shouldn't have to communicate with any raw webgpu here ideally...
#include "texture_holder.h"
#include "compute_shader.h"

#include <glm/glm.hpp>

using namespace glm;

class MarchingCubesManager {
    private:

    uint textureResolution;
    ComputeShader meshGeneratorShader;
};