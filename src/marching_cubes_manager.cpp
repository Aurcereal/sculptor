#include "marching_cubes_manager.h"

void testMouseClickCallback(vec2 pos) {
    std::cout << "CLICKED: " << pos.x << " " << pos.y << std::endl;
}

void testMouseMoveCallback(vec2 delta) {
    std::cout << "MOUSE MOVED: " << delta.x << " " << delta.y << std::endl;
}

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, InputManager *im, TextureFormat screenFormat) : 
    device(*d), queue(*q), inputManager(im), camera(), uniformManager(this), fieldEditor(this), meshGenerator(this), drawer(this) 
{
    inputManager->AddOnMouseClickListener(std::bind(&testMouseClickCallback, std::placeholders::_1));
    inputManager->AddOnMouseMoveListener(std::bind(&testMouseMoveCallback, std::placeholders::_1));

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

void MarchingCubes::Manager::Destroy() {
    uniformManager.Destroy();
    drawer.Destroy();
    meshGenerator.Destroy();
    fieldEditor.Destroy();
}