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
#include "input_manager.h"
#include "gui_manager.h"

using namespace glm;

namespace MarchingCubes {

    class Manager;

    // https://eliemichel.github.io/WebGPU-AutoLayout/
    struct Parameters {
        uint32_t textureResolution;
        uint32_t marchingCubesResolution;
        float marchingCubesThreshold;
        float _pad0[1];
        mat4x4 boundingBoxTRS;
        mat4x4 boundingBoxInverseTRS;
        mat4x4 boundingBoxInverseTranspose;
    };

    struct BrushParameters {
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

    struct RaycastInputUniform {
        vec3 origin;
        float _pad0;
        vec3 direction;
        float _pad1;
        inline RaycastInputUniform(vec3 o, vec3 d) : origin(o), direction(d) {}
    };

    class UniformManager {
    public:
        inline UniformManager(Manager *m) : manager(m) {}

        void Initialize();
        bool initialized = false;
        
        // Update Everything
        void UpdateParameters();
        void UpdateBrushParameters();
        void UpdateCameraTimeParameters();

        // Update certain parts of the struct
        void UpdateBrushMult(float);
        void UpdateBrushSize(float);
        void UpdateBrushType(uint32);

        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
        void UpdateModelMatrix();
        void UpdateTime();

        void SetRaycastInput(vec3 origin, vec3 direction);

        void Destroy();

        BufferHolder parameterBuffer;
        BufferHolder brushParameterBuffer;
        BufferHolder cameraTimeUniformBuffer;
        BufferHolder raycastInputUniformBuffer;
    private:
        Manager *manager;
        
    };

    class GUIToParams {
    public:
        inline GUIToParams(Manager *m) : manager(m) {}

        void Initialize(GUIManager*);
        bool initialized = false;

        void UpdateCameraParameters();

        Parameters parameters;
        BrushParameters brushParameters;
        CameraTimeUniform cameraTimeParameters;
    private:
        Manager *manager;

        int selectedBrush = 0;
        int selectedOperation = -1;
        int selectedShading = 0;

        void MainLoop();
    };

    class Raycaster {
    public:
        inline Raycaster(Manager *m) : manager(m) {}

        void Initialize();
        bool initialized = false;

        void InitializeWithDependencies();

        void RayFieldIntersect(vec3 origin, vec3 direction);  
        BufferHolder intersectionBuffer;

        void ResetIntersectionBuffer();

    private:
        Manager *manager;

        ComputeShader computeIntersection;
    };

    class FieldEditor {
    public:
        inline FieldEditor(Manager *m) : manager(m) {}

        void Initialize();
        bool initialized = false;

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
        bool initialized = false;

        void GenerateMesh();

        void Destroy();

        BufferHolder vertexBuffer;
        BufferHolder indexBuffer;
        BufferHolder countBuffer;

    private:
        Manager *manager;

        ComputeShader meshGenerationShader;

        const uint32_t maxVertexCount = 4194304;
        const uint32_t maxTriangleCount = 2097152;

        void ResetCountBuffer();
    };

    class Drawer {
    public:
        inline Drawer(Manager *m) : manager(m) {}

        void Initialize(TextureFormat);
        bool initialized = false;

        void UpdateIndexCount();
        
        void Destroy();

        DepthTexture depthTextureHolder;
        Shader drawShader;
        BufferHolder indirectDrawArgs;

    private:
        Manager *manager;
        ComputeShader fillIndexCountShader;
        
    };

    class InputHandler {
    public:
        inline InputHandler(Manager *m) : manager(m) {}

        void OnLMouseClick(vec2);
        void OnLMouseRelease(vec2);
        void OnMouseMove(vec2 newPos, vec2 delta);

        bool mouseDown = false;

    private:
        Manager *manager;
    };

    class Manager {
    public:
        Manager(const Device*, const Queue*, InputManager*, TextureFormat screenFormat, GUIManager*);
        Device device;
        Queue queue;
        InputManager *inputManager;
        
        GUIToParams guiToParams;
        InputHandler inputHandler;
        UniformManager uniformManager;
        Raycaster raycaster;
        FieldEditor fieldEditor;
        MeshGenerator meshGenerator;
        Drawer drawer;

        Camera camera;

        float boundingBoxScale;

        void MainLoop();

        void Destroy();

    private:
        
    };

}