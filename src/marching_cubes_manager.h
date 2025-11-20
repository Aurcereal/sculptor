#pragma once

// We shouldn't have to communicate with any raw webgpu here ideally...
// But we will a bit for writing buffer stuff for now
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
        Manager(const Device*);
        Device device;

        Raycaster raycaster;
        FieldEditor fieldEditor;
        MeshGenerator meshGenerator;
        Drawer drawer;
        Parameters parameters;

        private:

        uint textureResolution;
        ComputeShader meshGeneratorShader;
        
    };

    struct Parameters {
        uvec3 resolution;
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
        inline FieldEditor(Manager *m) : manager(m) {}
        
        void Initialize();
        void GenerateField();
        // void UpdateField();

    private:
        Manager *manager;

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