#pragma once

// We shouldn't have to communicate with any raw webgpu here ideally...
// But we will a bit for writing buffer stuff for now
#include "texture_holder.h"
#include "compute_shader.h"

#include <glm/glm.hpp>

#include "texture_holder.h"
#include "webgpu_utils.h"
#include "compute_shader.h"
#include "shader.h"
#include "camera.h"

using namespace glm;

namespace MarchingCubes {

    class Manager;

    struct Parameters {
        uint textureResolution;
        uint marchingCubesResolution;
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

        TextureHolder fieldTexture;
        // TextureHolder fieldScratchTexture;

        void Destroy();

    private:
        Manager *manager;

        ComputeShader fieldTextureInitializer;
        // ComputeShader fieldTextureDrawer;
        // ComputeShader copybackShader; // Copy fieldScratchTexture -> fieldTexture
        
    };

    class MeshGenerator {
    public:
        inline MeshGenerator(Manager *m) : manager(m) {}

        void Initialize();
        void GenerateMesh();

        void Destroy();

        BufferHolder vertexBuffer;
        BufferHolder indexBuffer;
        BufferHolder countBuffer;

    private:
        Manager *manager;

        ComputeShader meshGenerationShader;

        const uint32_t maxVertexCount = 262144;
        const uint32_t maxTriangleCount = 262144;

        void ResetCountBuffer();
    };

    class Drawer {
    public:
        inline Drawer(Manager *m) : manager(m) {}

        void Initialize(TextureFormat);
        void UpdateUniforms();
        void UpdateIndexCount();
        
        void Destroy();

        DepthTexture depthTextureHolder;
        Shader drawShader;
        BufferHolder indirectDrawArgs;

    private:
        Manager *manager;
        
        BufferHolder uniformBuffer;

        ComputeShader fillIndexCountShader;
        
    };

    class Manager {
        public:
        Manager(const Device*, const Queue*, TextureFormat screenFormat);
        Device device;
        Queue queue;

        Raycaster raycaster;
        FieldEditor fieldEditor;
        MeshGenerator meshGenerator;
        Drawer drawer;
        Parameters parameters;

        Camera camera;

        void MainLoop();
        void Destroy();

        private:

        ComputeShader meshGeneratorShader;
        
    };

}