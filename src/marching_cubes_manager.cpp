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

    fieldEditor.GenerateSphereField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::MainLoop() {
    guiToParams.UpdateCameraParameters();
    uniformManager.UpdateViewMatrix();
    uniformManager.UpdateTime();

    if(inputHandler.mouseDown) {
        raycaster.RayFieldIntersect(camera.pos, camera.Raycast(inputManager->mousePosition), continuouslyUpdateBrushOrientation);
    }
    inputHandler.HandleUpdates();

    fieldEditor.UpdateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::Destroy() {
    uniformManager.Destroy();
    drawer.Destroy();
    meshGenerator.Destroy();
    fieldEditor.Destroy();
}