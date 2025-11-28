#include "marching_cubes_manager.h"
#include <array>

using namespace ImGui;;
using namespace MarchingCubes;

void MarchingCubes::GUIToParams::Initialize(GUIManager *guiManager) {
    mat4x4 bbxTRS = glm::scale(mat4(1.0f), vec3(manager->boundingBoxScale)) * glm::translate(mat4(1.0f), vec3(-0.5f));
    mat4x4 bbxInverseTranspose = glm::transpose(glm::inverse(bbxTRS));
    mat4x4 bbxInvTRS = glm::translate(mat4(1.0f), vec3(0.5f)) * glm::scale(mat4(1.0f), vec3(1.0f/manager->boundingBoxScale));
    parameters = {256, 32, 0.1f, 0, bbxTRS, bbxInvTRS, bbxInverseTranspose, 0, 0};
    brushParameters = {0, .02f, .9f, 0.0f, vec3(0.3f,0.3f,0.4f), 0, 0, 0};
    cameraTimeParameters = {manager->camera.GetProjectionMatrix(), manager->camera.GetViewMatrix(), mat4(1.0f), 0.0f};

    guiManager->AddUIFunction(std::bind(&GUIToParams::MainLoop, this));

    initialized = true;
}

void MarchingCubes::GUIToParams::UpdateCameraParameters() {
    cameraTimeParameters = {manager->camera.GetProjectionMatrix(), manager->camera.GetViewMatrix(), mat4(1.0f), static_cast<float>(glfwGetTime())};
}

void MarchingCubes::GUIToParams::MainLoop() {
    ImGui::Begin("Controls");

    ImGui::SeparatorText("Brush Options");

    bool mirrorX = manager->guiToParams.parameters.mirrorX == 1;
    if(Checkbox("Mirror X", &mirrorX)) {
        manager->guiToParams.parameters.mirrorX = mirrorX ? 1 : 0;
        manager->uniformManager.UpdateParameters();
    }

    bool paintMode = manager->guiToParams.parameters.paintMode == 1;
    if(Checkbox("Paint Mode", &paintMode)) {
        manager->guiToParams.parameters.paintMode = paintMode ? 1 : 0;
        manager->uniformManager.UpdateParameters();
    }

    if(Button(("Brush Type: " + brushNames[brushParameters.brushType]).c_str()))
        OpenPopup("select_brush_popup");
    if(BeginPopup("select_brush_popup")) {
        SeparatorText("Brush Type");
        for(uint32 i=0; i<brushNames.size(); i++) {
            if(Selectable(brushNames[i].c_str())) {
                brushParameters.brushType = i;
                manager->uniformManager.UpdateBrushParameters();
            }
        }
        EndPopup();
    }

    if(brushParameters.brushType == 0 && CollapsingHeader("Draw Options")) {
        Indent();
        if(Button(("Brush Shape: " + drawShapes[brushParameters.drawShape]).c_str()))
            OpenPopup("select_draw_shape_popup");
        if(BeginPopup("select_draw_shape_popup")) {
            SeparatorText("Separator");
            for(uint32 i=0; i<drawShapes.size(); i++) {
                if(Selectable(drawShapes[i].c_str())) {
                    brushParameters.drawShape = i;
                    manager->uniformManager.UpdateBrushParameters();
                }
            }
            EndPopup();
        }

        if(Button(("Paint Texture: " + paintTextures[brushParameters.paintTexture]).c_str()))
            OpenPopup("select_paint_texture_popup");
        if(BeginPopup("select_paint_texture_popup")) {
            SeparatorText("Separator");
            for(uint32 i=0; i<paintTextures.size(); i++) {
                if(Selectable(paintTextures[i].c_str())) {
                    brushParameters.paintTexture = i;
                    manager->uniformManager.UpdateBrushParameters();
                }
            }
            EndPopup();
        }

        if(Button(("Sculpt Texture: " + sculptTextures[brushParameters.sculptTexture]).c_str()))
            OpenPopup("select_sculpt_texture_popup");
        if(BeginPopup("select_sculpt_texture_popup")) {
            SeparatorText("Separator");
            for(uint32 i=0; i<sculptTextures.size(); i++) {
                if(Selectable(sculptTextures[i].c_str())) {
                    brushParameters.sculptTexture = i;
                    manager->uniformManager.UpdateBrushParameters();
                }
            }
            EndPopup();
        }
        Unindent();
    }

    if(SliderFloat("Brush Size", &brushParameters.brushSize, 0.1f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic))
        manager->uniformManager.UpdateBrushParameters();
    if(SliderFloat("Brush Power", &brushParameters.brushMult, 0.001f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic))
        manager->uniformManager.UpdateBrushParameters();
    if(SliderFloat("Brush Hardness", &brushParameters.brushHardness, 0.001f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic))
        manager->uniformManager.UpdateBrushParameters();

    array<float, 3> cols = { brushParameters.color.r, brushParameters.color.g, brushParameters.color.b };
    if(ColorEdit3("Brush Color", cols.data())) {
        brushParameters.color = vec3(cols[0], cols[1], cols[2]);
        manager->uniformManager.UpdateBrushParameters();
    }

    ImGui::SeparatorText("Operations");

    const array<char*, 2> operationNames = {"Operation A", "Operation B"};
    if(ImGui::Button(selectedOperation == -1 ? "Select Operation" : operationNames[selectedOperation]))
        ImGui::OpenPopup("select_operation_popup");
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

    if(Button("Reset Object"))
        OpenPopup("reset_object_popup");
    if(BeginPopup("reset_object_popup")) {
        SeparatorText("Separator");
        for(int i=0; i<initializeShapeObjects.size(); i++) {
            if(Selectable(initializeShapeObjects[i].c_str())) {
                switch(static_cast<InitializeShapeObjects>(i)) {
                    case ISPHERE:
                        manager->fieldEditor.GenerateSphereField();
                        break;
                    case ICUBE:
                        manager->fieldEditor.GenerateCubeField();
                        break;
                    case IPLANE:
                        manager->fieldEditor.GeneratePlaneField();
                        break;
                }
            }
        }
        EndPopup();
    }


    ImGui::SeparatorText("Settings");

    const array<char*, 2> shadingNames = {"Smooth Shading", "Flat Shading"};
    if(ImGui::Button(shadingNames[manager->guiToParams.parameters.flatShading]))
        ImGui::OpenPopup("select_shading_popup");
    if(ImGui::BeginPopup("select_shading_popup")) {
        ImGui::SeparatorText("Separator");
        for(int i=0; i<shadingNames.size(); i++) {
            if(ImGui::Selectable(shadingNames[i])) {
                manager->guiToParams.parameters.flatShading = i;
                manager->uniformManager.UpdateParameters();
            }
        }
        ImGui::EndPopup();
    }

    int mr = static_cast<int>(parameters.marchingCubesResolution);
    if(ImGui::SliderInt("Marching Cubes Resolution", &mr, 8, 64)) {
        std::cout << "res chang" << std::endl;
        parameters.marchingCubesResolution = static_cast<uint32_t>(mr);
        manager->uniformManager.UpdateParameters();
    }

    ImGui::End();
}