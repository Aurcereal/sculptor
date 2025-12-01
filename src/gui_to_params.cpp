#include "marching_cubes_manager.h"
#include <array>

using namespace ImGui;;
using namespace MarchingCubes;

void MarchingCubes::GUIToParams::Initialize(GUIManager *guiManager) {
    mat4x4 bbxTRS = glm::scale(mat4(1.0f), vec3(manager->boundingBoxScale)) * glm::translate(mat4(1.0f), vec3(-0.5f));
    mat4x4 bbxInverseTranspose = glm::transpose(glm::inverse(bbxTRS));
    mat4x4 bbxInvTRS = glm::translate(mat4(1.0f), vec3(0.5f)) * glm::scale(mat4(1.0f), vec3(1.0f/manager->boundingBoxScale));
    parameters = {412, 32, 0.1f, 0, bbxTRS, bbxInvTRS, bbxInverseTranspose, 0, 0, 0};
    brushParameters = {0, .02f, .9f, 0.0f, vec3(0.3f,0.3f,0.4f), 0, 0, 0, 1};
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

    bool paintMode = parameters.paintMode == 1;
    if(Checkbox("Paint Mode", &paintMode)) {
        parameters.paintMode = paintMode ? 1 : 0;
        brushParameters.brushHardness = 0.0f;
        manager->uniformManager.UpdateParameters();
        manager->uniformManager.UpdateBrushParameters();
    }

    if(Button(("Brush Type: " + brushNames[brushParameters.brushType]).c_str()))
        OpenPopup("select_brush_popup");
    if(BeginPopup("select_brush_popup")) {
        SeparatorText("Brush Type");
        for(uint32 i=0; i<brushNames.size(); i++) {
            if(Selectable(brushNames[i].c_str())) {
                brushParameters.brushType = i;
                if(i == 1) {
                    // Reset Draw Settings
                    brushParameters.drawShape = 0;
                    brushParameters.paintTexture = 0;
                    brushParameters.sculptTexture = 0; 
                }
                manager->uniformManager.UpdateBrushParameters();
            }
        }
        EndPopup();
    }

    if(brushParameters.brushType == 0/* && CollapsingHeader("Draw Options")*/) {
        Indent();
        bool followNormal = brushParameters.brushFollowNormal == 1;
        if(Checkbox("Align Draw Shape w/ Normal", &followNormal)) {
            brushParameters.brushFollowNormal = followNormal ? 1 : 0;
            manager->uniformManager.UpdateBrushParameters();
        }

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

        if(!parameters.paintMode) {
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
        }
        Unindent();
    }

    if(SliderFloat("Brush Size", &brushParameters.brushSize, 0.1f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic))
        manager->uniformManager.UpdateBrushParameters();
    if(SliderFloat("Brush Power", &brushParameters.brushMult, 0.001f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic))
        manager->uniformManager.UpdateBrushParameters();
    if(parameters.paintMode) {
        if(SliderFloat("Brush Hardness", &brushParameters.brushHardness, 0.001f, 1.0f, "%.4f", ImGuiSliderFlags_Logarithmic))
            manager->uniformManager.UpdateBrushParameters();
    }

    array<float, 3> cols = { brushParameters.color.r, brushParameters.color.g, brushParameters.color.b };
    if(ColorEdit3("Brush Color", cols.data())) {
        brushParameters.color = vec3(cols[0], cols[1], cols[2]);
        manager->uniformManager.UpdateBrushParameters();
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
                    case IREDPLANET:
                        manager->fieldEditor.GenerateRedPlanetField();
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
    if(ImGui::SliderInt("Mesh LoD", &mr, 8, 112)) {
        std::cout << "res chang" << std::endl;
        parameters.marchingCubesResolution = static_cast<uint32_t>(mr);
        manager->uniformManager.UpdateParameters();
    }

    if(CollapsingHeader("Advanced")) {
        Indent();
        Checkbox("Update Brush Direction While Pressed", &manager->continuouslyUpdateBrushOrientation);

        bool leveledMode = manager->guiToParams.parameters.leveledMode == 1;
        if(Checkbox("Leveled Mode", &leveledMode)) {
            manager->guiToParams.parameters.leveledMode = leveledMode ? 1 : 0;
            manager->uniformManager.UpdateParameters();
        }

        Unindent();
    }

    ImGui::End();
}