#pragma once

// We shouldn't have to communicate with any raw webgpu here ideally...
#include "texture_holder.h"
#include "compute_shader.h"

#include <glm/glm.hpp>

using namespace glm;

namespace MarchingCubes {

    class Manager {
        public:
        Raycaster raycaster;
        FieldEditor fieldEditor;
        MeshGenerator meshGenerator;
        Drawer drawer;

        private:

        uint textureResolution;
        ComputeShader meshGeneratorShader;
        
    };

    class Raycaster {
        struct Intersection {
            vec3 pos;
            vec3 norm;
        };

        public:
        Intersection RayFieldIntersect(vec3 origin, vec3 direction);  
    };

    class FieldEditor {
    };

    class MeshGenerator {
    };

    class Drawer {
        // Next make a shader class to put in here, initializerenderpipeline
        // will migrate to the class and do a very similar shaderparameter thing
        // like i did inc ompute shader
    };

}