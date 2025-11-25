#pragma once

#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <imgui.h>
#include <vector>
#include <functional>

using namespace std;
using namespace wgpu;

class GUIManager {
public:
    void Initialize(Device&, GLFWwindow*, TextureFormat);
    void DrawAndUpdateGUI(RenderPassEncoder);
    void Terminate();
    void AddUIFunction(const std::function<void()>&);

private:
    vector<std::function<void()>> buildUIFunctions;
};