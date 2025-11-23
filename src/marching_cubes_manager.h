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

    // https://eliemichel.github.io/WebGPU-AutoLayout/
    struct Parameters {
        uint32_t textureResolution;
        uint32_t marchingCubesResolution;
        float _pad0[2];
    };

    struct BrushParameters {
        mat4x4 brushInverseTR;
        uint32 brushType;
        float brushMult;
        float brushSize;
        float _pad0[1];
    };

    struct CameraTimeUniform {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        mat4x4 modelMatrix;
        float time;
        float _pad[3];
    };

    class UniformManager {
    public:
        inline UniformManager(Manager *m) : manager(m) {}

        void Initialize(uint32 textureResolution, uint32 marchingCubesResolution, const Camera&);
        
        void UpdateResolutions(const uint32 *newTexResolution, const uint32 *newMarchingCubesResolution);

        void UpdateBrushTransform(const mat4x4&);
        void UpdateBrushMult(float);
        void UpdateBrushSize(float);
        void UpdateBrushType(uint32);

        void UpdateViewMatrix(const Camera&);
        void UpdateProjectionMatrix(const Camera&);
        void UpdateModelMatrix(const mat4x4&);
        void UpdateTime();

        void Destroy();

        BufferHolder parameterBuffer;
        BufferHolder brushParameterBuffer;
        BufferHolder cameraTimeUniformBuffer;

        Parameters parameters;
        BrushParameters brushParameters;
        CameraTimeUniform cameraTimeParameters;
    private:
        Manager *manager;
        
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
        void UpdateField();

        TextureHolder fieldTexture;
        TextureHolder fieldScratchTexture;

        void Destroy();

    private:
        Manager *manager;

        ComputeShader fieldInitializer;
        ComputeShader copybackShader; 
        void CopyScratchToField(); // Copy fieldScratchTexture -> fieldTexture

        ComputeShader fieldDrawUpdater;
        
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
        void UpdateIndexCount();
        
        void Destroy();

        DepthTexture depthTextureHolder;
        Shader drawShader;
        BufferHolder indirectDrawArgs;

    private:
        Manager *manager;
        ComputeShader fillIndexCountShader;
        
    };

    class Manager {
        public:
        Manager(const Device*, const Queue*, TextureFormat screenFormat);
        Device device;
        Queue queue;

        UniformManager uniformManager;
        Raycaster raycaster;
        FieldEditor fieldEditor;
        MeshGenerator meshGenerator;
        Drawer drawer;

        Camera camera;

        void MainLoop();
        void Destroy();

        private:

        ComputeShader meshGeneratorShader;
        
    };

}