#include "marching_cubes_manager.h"

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, InputManager *im, TextureFormat screenFormat) : 
    device(*d), queue(*q), inputManager(im), camera(), uniformManager(this), raycaster(this), fieldEditor(this), meshGenerator(this), drawer(this),
    boundingBoxScale(4.0f)
{
    mat4x4 bbxTRS = glm::rotate(mat4(1.0f), 0.4f, vec3(1,0,0)) * glm::scale(mat4(1.0f), vec3(boundingBoxScale)) * glm::translate(mat4(1.0f), vec3(-0.5f));
    mat4x4 bbxInverseTranspose = glm::transpose(glm::inverse(bbxTRS));
    mat4x4 bbxInvTRS = glm::translate(mat4(1.0f), vec3(0.5f)) * glm::scale(mat4(1.0f), vec3(1.0f/boundingBoxScale)) * glm::rotate(mat4(1.0f), -0.4f, vec3(1,0,0));
    parameters = {256, 32, 0.5f, -1.0f, bbxTRS, bbxInvTRS, bbxInverseTranspose};

    inputManager->AddOnMouseClickListener(std::bind(&MarchingCubes::Manager::OnMouseClick, this, std::placeholders::_1));
    inputManager->AddOnMouseMoveListener(std::bind(&MarchingCubes::Manager::OnMouseMove, this, std::placeholders::_1));

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
    uniformManager.UpdateModelMatrix(glm::rotate(mat4(1.0f), static_cast<float>(glfwGetTime()), vec3(0.0f, 1.0f, 0.0f)));
    uniformManager.UpdateViewMatrix(camera);
    uniformManager.UpdateTime();

    fieldEditor.UpdateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::OnMouseClick(vec2 pos) {
    vec3 rd = camera.Raycast(pos);
    std::cout << "CLICKED: " << rd.x << " " << rd.y << " " << rd.z << std::endl;
}
void MarchingCubes::Manager::OnMouseMove(vec2 delta) {
    std::cout << "MOUSE MOVED: " << delta.x << " " << delta.y << std::endl;
}

void MarchingCubes::Manager::Destroy() {
    uniformManager.Destroy();
    drawer.Destroy();
    meshGenerator.Destroy();
    fieldEditor.Destroy();
}