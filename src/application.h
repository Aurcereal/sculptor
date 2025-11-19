
#pragma once

#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <utility>
#include <glm/glm.hpp>

#include "camera.h"
#include "texture_holder.h"
#include "compute_shader.h"

using namespace wgpu;
using namespace glm;

class Application {
    public:
    
    inline Application() : device(nullptr), queue(nullptr), surface(nullptr), pipeline(nullptr), pipelineLayout(nullptr) {}
    bool Initialize(); // Was initialization succesful?
    void Terminate();
    void MainLoop(); // Draw frame and handle events
    bool IsRunning(); // Returns true as long as main loop should keep going

    inline Device& GetDevice() { return device; }
    inline Queue& GetQueue() { return queue; }

    private:
    void testComputeMeshGenerate();
    Camera camera;

    struct MyUniforms {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        mat4x4 modelMatrix;
        vec4 color;
        float time;
        float _pad[3];
    };
    static_assert(sizeof(MyUniforms) % 16 == 0);
    
    std::pair<SurfaceTexture, TextureView> GetNextSurfaceViewData();
    void InitializeRenderPipeline();
    RequiredLimits GetRequiredLimits(Adapter adapter) const;
    
    GLFWwindow *window;
    Device device;
    Queue queue;
    Surface surface;
    TextureFormat surfaceFormat = TextureFormat::Undefined;

    // RenderPipeline pipeline;
    // PipelineLayout pipelineLayout;
    void InitializeBindGroups();
    // BindGroupLayout bindGroupLayout = nullptr;
    // BindGroup bindGroup = nullptr; // Bind Group mirrors the Layout, contains actual bindings
    Texture depthTexture = nullptr;
    TextureView depthTextureView = nullptr;

    void InitializeBuffers();
    BufferHolder vertexBuffer;
    BufferHolder indexBuffer;
    BufferHolder countBuffer;
    Buffer uniformBuffer = nullptr;
    uint32_t indexCount;

    ComputeShader testComputeShader;
    TextureHolder testInputTexture;
    TextureHolder testTexture;
};