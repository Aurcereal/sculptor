#include "gui_manager.h"
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>

void GUIManager::Initialize(Device& device, GLFWwindow *window, TextureFormat swapChainFormat) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO();

    // Platform/Renderer specific backends
    ImGui_ImplGlfw_InitForOther(window, true);
    ImGui_ImplWGPU_Init(device, 3, swapChainFormat, TextureFormat::Depth24Plus);
}
void GUIManager::DrawAndUpdateGUI(RenderPassEncoder renderPass) {
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //
    for(auto& f : buildUIFunctions) {
        f();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
}
void GUIManager::Terminate() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
}
void GUIManager::AddUIFunction(const std::function<void()>& f) {
    buildUIFunctions.push_back(f);
}