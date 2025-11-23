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
#include "shader.h"
#include "depth_texture.h"
#include "marching_cubes_manager.h"
#include "uPtr_macros.h"
#include "input_manager.h"

using namespace wgpu;
using namespace glm;

class Application {
    public:
    
    inline Application() : device(nullptr), queue(nullptr), surface(nullptr) {}
    bool Initialize(); // Was initialization succesful?
    void Terminate();
    void MainLoop(); // Draw frame and handle events
    bool IsRunning(); // Returns true as long as main loop should keep going

    inline Device& GetDevice() { return device; }
    inline Queue& GetQueue() { return queue; }

    uvec2 resolution;

    private:
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
    RequiredLimits GetRequiredLimits(Adapter adapter) const;
    
    GLFWwindow *window;
    Device device;
    Queue queue;
    Surface surface;
    TextureFormat surfaceFormat = TextureFormat::Undefined;

    //
    uPtr<MarchingCubes::Manager> marchingCubesManager;
    InputManager inputManager;
};