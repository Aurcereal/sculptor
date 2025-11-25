#include "marching_cubes_manager.h"

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, InputManager *im, TextureFormat screenFormat, GUIManager *guiManager) : 
    device(*d), queue(*q), inputManager(im), camera(im), guiToParams(this), inputHandler(this), uniformManager(this), raycaster(this), fieldEditor(this), meshGenerator(this), drawer(this),
    boundingBoxScale(4.0f)
{
    inputManager->AddOnMouseClickListener(std::bind(&MarchingCubes::InputHandler::OnLMouseClick, &inputHandler, std::placeholders::_1));
    inputManager->AddOnMouseReleaseListener(std::bind(&MarchingCubes::InputHandler::OnLMouseRelease, &inputHandler, std::placeholders::_1));
    inputManager->AddOnMouseMoveListener(std::bind(&MarchingCubes::InputHandler::OnMouseMove, &inputHandler, std::placeholders::_1, std::placeholders::_2));

    guiToParams.Initialize(guiManager); // Initialize FIRST
    uniformManager.Initialize(); // Initialize SECOND using FIRST

    raycaster.Initialize();
    fieldEditor.Initialize();
    meshGenerator.Initialize();
    drawer.Initialize(screenFormat); 

    raycaster.InitializeWithDependencies();

    fieldEditor.GenerateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::MainLoop() {
    guiToParams.UpdateCameraParameters();
    uniformManager.UpdateViewMatrix();
    uniformManager.UpdateTime();

    if(inputHandler.mouseDown) {
        raycaster.RayFieldIntersect(camera.pos, camera.Raycast(inputManager->mousePosition));
    }

    fieldEditor.UpdateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::InputHandler::OnLMouseClick(vec2 pos) {
    mouseDown = true;
    vec3 rd = manager->camera.Raycast(pos);
    std::cout << "CLICKED: " << rd.x << " " << rd.y << " " << rd.z << std::endl;
    manager->raycaster.RayFieldIntersect(manager->camera.pos, rd);
}
void MarchingCubes::InputHandler::OnLMouseRelease(vec2) {
    std::cout << "RELEASE" << std::endl;
    mouseDown = false;
    manager->raycaster.ResetIntersectionBuffer();
}
void MarchingCubes::InputHandler::OnMouseMove(vec2, vec2) {
    
}

void MarchingCubes::Manager::Destroy() {
    uniformManager.Destroy();
    drawer.Destroy();
    meshGenerator.Destroy();
    fieldEditor.Destroy();
}