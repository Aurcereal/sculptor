#include "marching_cubes_manager.h"

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, InputManager *im, TextureFormat screenFormat, GUIManager *guiManager) : 
    device(*d), queue(*q), inputManager(im), camera(im), 
    guiToParams(this), inputHandler(this), uniformManager(this), raycaster(this), cursorOperator(this), 
    fieldEditor(this), meshGenerator(this), drawer(this),
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
        vec3 cursorPos;
        mat3x3 cursorFrame;
        raycaster.RayFieldIntersect(camera.pos, camera.Raycast(inputManager->mousePosition), 
            &cursorPos, &cursorFrame, continuouslyUpdateBrushOrientation);
        cursorOperator.Update(cursorPos, cursorFrame);
    }
    inputHandler.HandleUpdates();

    fieldEditor.UpdateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::CursorOperator::Update(vec3 pos, mat3x3 frame) {
    vec3 delta = pos - prevPos;
    vec3 lDelta = transpose(frame) * delta;
    currOffset += vec2(lDelta.x, lDelta.y);

    std::cout << "X: " << currOffset.x << " Y: " << currOffset.y << std::endl;

    prevPos = pos;
    prevFrame = frame;
}

void MarchingCubes::CursorOperator::OnClick(vec3 pos, mat3x3 frame) {
    currOffset = vec2(0.0f);
    prevPos = pos;
    prevFrame = frame;
}

void MarchingCubes::Manager::Destroy() {
    uniformManager.Destroy();
    drawer.Destroy();
    meshGenerator.Destroy();
    fieldEditor.Destroy();
}