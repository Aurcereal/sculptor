#include "marching_cubes_manager.h"

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, InputManager *im, TextureFormat screenFormat) : 
    device(*d), queue(*q), inputManager(im), camera(im), inputHandler(this), uniformManager(this), raycaster(this), fieldEditor(this), meshGenerator(this), drawer(this),
    boundingBoxScale(4.0f)
{
    inputManager->AddOnMouseClickListener(std::bind(&MarchingCubes::InputHandler::OnLMouseClick, &inputHandler, std::placeholders::_1));
    inputManager->AddOnMouseReleaseListener(std::bind(&MarchingCubes::InputHandler::OnLMouseRelease, &inputHandler, std::placeholders::_1));
    inputManager->AddOnMouseMoveListener(std::bind(&MarchingCubes::InputHandler::OnMouseMove, &inputHandler, std::placeholders::_1, std::placeholders::_2));

    mat4x4 bbxTRS = glm::rotate(mat4(1.0f), 0.4f, vec3(1,0,0)) * glm::scale(mat4(1.0f), vec3(boundingBoxScale)) * glm::translate(mat4(1.0f), vec3(-0.5f));
    mat4x4 bbxInverseTranspose = glm::transpose(glm::inverse(bbxTRS));
    mat4x4 bbxInvTRS = glm::translate(mat4(1.0f), vec3(0.5f)) * glm::scale(mat4(1.0f), vec3(1.0f/boundingBoxScale)) * glm::rotate(mat4(1.0f), -0.4f, vec3(1,0,0));
    parameters = {256, 32, 0.5f, -1.0f, bbxTRS, bbxInvTRS, bbxInverseTranspose};

    raycaster.Initialize();
    uniformManager.Initialize(parameters, camera);
    fieldEditor.Initialize();
    meshGenerator.Initialize();
    drawer.Initialize(screenFormat); 

    raycaster.InitializeWithDependencies();

    fieldEditor.GenerateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::MainLoop() {
    //uniformManager.UpdateModelMatrix(glm::rotate(mat4(1.0f), static_cast<float>(glfwGetTime()), vec3(0.0f, 1.0f, 0.0f)));
    uniformManager.UpdateViewMatrix(camera);
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