#include "marching_cubes_manager.h"

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, InputManager *im, TextureFormat screenFormat) : 
    device(*d), queue(*q), inputManager(im), camera(), uniformManager(this), raycaster(this), fieldEditor(this), meshGenerator(this), drawer(this) 
{
    inputManager->AddOnMouseClickListener(std::bind(&MarchingCubes::Manager::OnMouseClick, this, std::placeholders::_1));
    inputManager->AddOnMouseMoveListener(std::bind(&MarchingCubes::Manager::OnMouseMove, this, std::placeholders::_1));

    uniformManager.Initialize(256, 16, camera);

    fieldEditor.Initialize();
    meshGenerator.Initialize();
    drawer.Initialize(screenFormat); 

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