#include "marching_cubes_manager.h"

MarchingCubes::Manager::Manager(const Device* d, const Queue* q, TextureFormat screenFormat) : 
    device(*d), queue(*q), fieldEditor(this), meshGenerator(this), drawer(this) 
{
    parameters = {
        256, // Texture
        16   // Marching Cubes
    };

    fieldEditor.Initialize();
    meshGenerator.Initialize();
    drawer.Initialize(screenFormat); 

    fieldEditor.GenerateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::MainLoop() {
    drawer.UpdateUniforms();

    fieldEditor.UpdateField();
    meshGenerator.GenerateMesh();
    drawer.UpdateIndexCount();
}

void MarchingCubes::Manager::Destroy() {
    drawer.Destroy();
    meshGenerator.Destroy();
    fieldEditor.Destroy();
}