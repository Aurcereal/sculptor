
#include "marching_cubes_manager.h"

using namespace std;
namespace SP = ShaderParameter;

void MarchingCubes::FieldEditor::Initialize() {
    fieldTexture.Initialize(manager->device, uvec3(manager->parameters.textureResolution), TextureFormat::R32Float, true, true);
    fieldScratchTexture.Initialize(manager->device, uvec3(manager->parameters.textureResolution), TextureFormat::R32Float, true, true);

    //
    vector<ShaderParameter::Parameter> params = {
        SP::Parameter(SP::UTexture{&fieldScratchTexture, true, true}) // TEMP, make it fieldTexture then the update will write to scratch
    };
    fieldTextureInitializer.Initialize(manager->device, params, "/field_editor/initialize_field.wgsl");

    //
    vector<ShaderParameter::Parameter> copybackParams = {
        SP::Parameter(SP::UTexture{&fieldScratchTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldTexture, true, true})
    };
    copybackShader.Initialize(manager->device, copybackParams, "./field_editor/copy_back.wgsl");

    std::cout << "Initialized Field Editor" << std::endl;
}

void MarchingCubes::FieldEditor::GenerateField() {
    fieldTextureInitializer.DispatchSync(manager->device, manager->queue, uvec3(manager->parameters.textureResolution));
    CopyScratchToField();
}

void MarchingCubes::FieldEditor::Destroy() {
    fieldTexture.Destroy();
    fieldScratchTexture.Destroy();
    fieldTextureInitializer.Destroy();
    copybackShader.Destroy();
}

void MarchingCubes::FieldEditor::CopyScratchToField() {
    copybackShader.DispatchSync(manager->device, manager->queue, uvec3(manager->parameters.textureResolution));
}