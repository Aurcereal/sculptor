#include "marching_cubes_manager.h"
#include <array>

void MarchingCubes::GUIToParams::Initialize(GUIManager *guiManager) {
    mat4x4 bbxTRS = glm::rotate(mat4(1.0f), 0.4f, vec3(1,0,0)) * glm::scale(mat4(1.0f), vec3(manager->boundingBoxScale)) * glm::translate(mat4(1.0f), vec3(-0.5f));
    mat4x4 bbxInverseTranspose = glm::transpose(glm::inverse(bbxTRS));
    mat4x4 bbxInvTRS = glm::translate(mat4(1.0f), vec3(0.5f)) * glm::scale(mat4(1.0f), vec3(1.0f/manager->boundingBoxScale)) * glm::rotate(mat4(1.0f), -0.4f, vec3(1,0,0));
    parameters = {256, 32, 0.5f, -1.0f, bbxTRS, bbxInvTRS, bbxInverseTranspose};
    brushParameters = {0, 1.0f, 1.0f};
    cameraTimeParameters = {manager->camera.GetProjectionMatrix(), manager->camera.GetViewMatrix(), mat4(1.0f), 0.0f};

    guiManager->AddUIFunction(std::bind(&GUIToParams::MainLoop, this));

    initialized = true;
}

void MarchingCubes::GUIToParams::UpdateCameraParameters() {
    cameraTimeParameters = {manager->camera.GetProjectionMatrix(), manager->camera.GetViewMatrix(), mat4(1.0f), static_cast<float>(glfwGetTime())};
}

void MarchingCubes::GUIToParams::MainLoop() {
    ImGui::Begin("Controls");

    ImGui::SeparatorText("Brush");
    const array<char*, 3> brushNames = { "Brush A", "Brush B", "Bruch C"};
    ImGui::TextWrapped("Test text...");
    if(ImGui::Button(selectedBrush == -1 ? "Select Brush" : brushNames[selectedBrush]))
        ImGui::OpenPopup("select_brush_popup");
    //ImGui::SameLine();
    //ImGui::TextUnformatted(selectedBrush == -1 ? "No Brush Selected" : brushNames[selectedBrush]);
    if(ImGui::BeginPopup("select_brush_popup")) {
        ImGui::SeparatorText("Separator");
        for(int i=0; i<brushNames.size(); i++) {
            if(ImGui::Selectable(brushNames[i])) {
                selectedBrush = i;
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SeparatorText("Operations");

    const array<char*, 2> operationNames = {"Operation A", "Operation B"};
    if(ImGui::Button(selectedOperation == -1 ? "Select Operation" : operationNames[selectedOperation]))
        ImGui::OpenPopup("select_operation_popup");
    // ImGui::SameLine();
    // ImGui::TextUnformatted(selectedOperation == -1 ? "No Operation Selected" : operationNames[selectedOperation]);
    if(ImGui::BeginPopup("select_operation_popup")) {
        ImGui::SeparatorText("Separator");
        for(int i=0; i<operationNames.size(); i++) {
            if(ImGui::Selectable(operationNames[i])) {
                selectedOperation = i;
            }
        }
        ImGui::EndPopup();
    }
    switch(selectedOperation) {
        case 0:
        ImGui::TextWrapped("Operation 0 settings here..");
        break;
        case 1:
        ImGui::TextWrapped("Operation 1 settings here...");
        break;
    }
    if(selectedOperation != -1) {
        ImGui::Button("Apply Operation");
    }

    ImGui::SeparatorText("Settings");

    //const array<char*, 2> shadingNames = {"Flat Shading", "Smooth Shading"};

    ImGui::End();
}