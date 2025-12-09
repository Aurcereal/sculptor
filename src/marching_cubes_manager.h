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
#include "buffer_helper.h"

#include <array>

using namespace glm;

namespace MarchingCubes {

    class Manager;

    // https://eliemichel.github.io/WebGPU-AutoLayout/ All of these structs need to have length multiple of 4 bytes
    struct Parameters {
        uint32_t textureResolution;
        uint32_t marchingCubesResolution;
        float marchingCubesThreshold;
        uint32 flatShading;
        mat4x4 boundingBoxTRS;
        mat4x4 boundingBoxInverseTRS;
        mat4x4 boundingBoxInverseTranspose;
        uint32 mirrorX;
        uint32 paintMode;
        uint32 leveledMode;
        float _padding0;
        vec3 lightDirection;
        float _padding1[1];
    };

    struct BrushParameters {
        uint32 brushType;
        float brushMult;
        float brushSize;
        float brushHardness;
        vec3 color;
        uint32 drawShape;
        uint32 paintTexture;
        uint32 sculptTexture;
        uint32 brushFollowNormal;
        float _padding0;
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
        uint32 writeNormal;
        inline RaycastInputUniform(vec3 o, vec3 d, bool wn = true) : origin(o), direction(d), writeNormal(wn ? 1 : 0) {}
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
        void UpdateBrushMult();
        void UpdateBrushSize();
        void UpdateBrushType();

        void UpdateViewMatrix();
        void UpdateProjectionMatrix();
        void UpdateModelMatrix();
        void UpdateTime();

        void SetRaycastInput(vec3 origin, vec3 direction, bool writeNormal);

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

        const array<string, 2> brushNames = {
            "Draw", "Twirl"
        };

        const array<string, 5> drawShapes = {
            "Sphere", "Cube", "Cone", "Triangle", "Star" 
        };
        const array<string, 7> paintTextures = {
            "Solid Color", "Swirly", "Polka Dot", "Stripes", "Checker", "Circle Pattern", "Noisy"
        };
        const array<string, 11> sculptTextures = { // Usually booleans
            "None", "Checker", "Polka Dot", "Sphere Pattern", "Gyroid", 
            "Noisy",
            "Comb", "Cube Comb", "Bumpy", "Voronoi", "Spiky"
        };

        const array<string, 4> initializeShapeObjects = {
            "Sphere", "Cube", "Plane", "Red Planet"
        };
        enum InitializeShapeObjects {
            ISPHERE, ICUBE, IPLANE, IREDPLANET
        };

        int selectedOperation = -1;

        void MainLoop();
    };

    class Raycaster {
    public:
        inline Raycaster(Manager *m) : manager(m) {}

        void Initialize();
        bool initialized = false;

        void InitializeWithDependencies();

        void RayFieldIntersect(vec3 origin, vec3 direction, bool writeNormal = true);
        void RayFieldIntersect(vec3 origin, vec3 direction, vec3 *outPosition, mat3x3 *outFrame, bool writeNormal = true);  
        BufferHolder intersectionBuffer;

        void ResetIntersectionBuffer();
        void SendOffset(vec2);

    private:
        Manager *manager;

        ComputeShader computeIntersection;
    };

    class CursorOperator {
    public:
        inline CursorOperator(Manager *m) : manager(m) {}

        void Update(vec3 newCursorPos, mat3x3 newCursorFrame);
        void OnClick(vec3 newCursorPos, mat3x3 newCursorFrame);

    private:
        vec3 prevPos;
        mat3x3 prevFrame;

        vec2 currOffset = vec2(0.0f);

        Manager *manager;

    };

    class FieldEditor {
    public:
        inline FieldEditor(Manager *m) : manager(m) {}

        void Initialize();
        bool initialized = false;

        void GenerateSphereField();
        void GenerateCubeField();
        void GeneratePlaneField();
        void GenerateRedPlanetField();

        void UpdateField();

        TextureHolder fieldTexture;
        TextureHolder fieldColorTexture;

        TextureHolder fieldScratchTexture;
        TextureHolder fieldColorScratchTexture;

        void Destroy();

    private:
        Manager *manager;

        ComputeShader sphereFieldInitializer;
        ComputeShader cubeFieldInitializer;
        ComputeShader planeFieldInitializer;
        ComputeShader redPlanetFieldInitializer;

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

        const uint32_t maxVertexCount = 33554432; // Almost a GB of Mesh Data..
        const uint32_t maxTriangleCount = 16777216;

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

        void HandleUpdates();

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
        CursorOperator cursorOperator;
        FieldEditor fieldEditor;
        MeshGenerator meshGenerator;
        Drawer drawer;

        Camera camera;

        float boundingBoxScale;
        bool continuouslyUpdateBrushOrientation = true;

        void MainLoop();

        void Destroy();

    private:
        
    };

}