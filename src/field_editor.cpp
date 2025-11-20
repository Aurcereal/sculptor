
#include "marching_cubes_manager.h"

using namespace std;
namespace SP = ShaderParameter;

void MarchingCubes::FieldEditor::Initialize() {
    fieldTexture.Initialize(manager->device, uvec3(manager->parameters.textureResolution), TextureFormat::R32Float, true, true);

    vector<ShaderParameter::Parameter> params = {
        SP::Parameter(SP::UTexture{&fieldTexture, true, true})
    };

    fieldTextureInitializer.Initialize(manager->device, params, "/field_editor/initialize_field.wgsl");

    std::cout << "Initialized Field Editor" << std::endl;
}

void MarchingCubes::FieldEditor::GenerateField() {
    bool finished = false;
    fieldTextureInitializer.Dispatch(manager->device, manager->queue, uvec3(manager->parameters.textureResolution), &finished);
    while(!finished) {
        wgpuPollEvents(manager->device, true);
    }
}

void MarchingCubes::FieldEditor::Destroy() {
    fieldTexture.Destroy();
    fieldTextureInitializer.Destroy();
}