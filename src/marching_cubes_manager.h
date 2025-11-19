#pragma once

// We shouldn't have to communicate with any raw webgpu here ideally...
#include "texture_holder.h"
#include "compute_shader.h"

#include <glm/glm.hpp>

#include "texture_holder.h"
#include "webgpu_utils.h"
#include "compute_shader.h"

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
    public:
        void Initialize();
        void GenerateField();
        // void UpdateField();

    private:
        ComputeShader fieldTextureInitializer;
        // ComputeShader fieldTextureDrawer;
        // ComputeShader copybackShader; // Copy fieldScratchTexture -> fieldTexture

        TextureHolder fieldTexture;
        // TextureHolder fieldScratchTexture;
    };

    class MeshGenerator {
    public:
        void Initialize();
        void GenerateMesh();

    private:
        ComputeShader meshGenerationShader;

        BufferHolder vertexBuffer;
        BufferHolder indexBuffer;
        BufferHolder countBuffer;
    };

    class Drawer {
    public:
        void Initialize();

        // shaders and such
        // headres in the same file, implementations in different files?
    };

}