
#include "marching_cubes_manager.h"

using namespace std;
namespace SP = ShaderParameter;

void MarchingCubes::FieldEditor::Initialize() {
    fieldTexture.Initialize(manager->device, uvec3(manager->uniformManager.parameters.textureResolution), TextureFormat::R32Float, true, true);
    fieldScratchTexture.Initialize(manager->device, uvec3(manager->uniformManager.parameters.textureResolution), TextureFormat::R32Float, true, true);

    //
    vector<ShaderParameter::Parameter> params = {
        SP::Parameter(SP::UTexture{&fieldTexture, true, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer})
    };
    fieldInitializer.Initialize(manager->device, params, "/field_editor/initialize_field.wgsl");

    //
    vector<ShaderParameter::Parameter> copybackParams = {
        SP::Parameter(SP::UTexture{&fieldScratchTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldTexture, true, true})
    };
    copybackShader.Initialize(manager->device, copybackParams, "./field_editor/copy_back.wgsl");
    
    //
    vector<ShaderParameter::Parameter> drawUpdateParams = {
        SP::Parameter(SP::UTexture{&fieldTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldScratchTexture, true, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.cameraTimeUniformBuffer})
    };
    fieldDrawUpdater.Initialize(manager->device, drawUpdateParams, "./field_editor/field_update_draw.wgsl");

    std::cout << "Initialized Field Editor" << std::endl;
}

void MarchingCubes::FieldEditor::GenerateField() {
    fieldInitializer.DispatchSync(manager->device, manager->queue, uvec3(manager->uniformManager.parameters.textureResolution));
}

void MarchingCubes::FieldEditor::UpdateField() {
    fieldDrawUpdater.DispatchSync(manager->device, manager->queue, uvec3(manager->uniformManager.parameters.textureResolution));
    CopyScratchToField();
}

void MarchingCubes::FieldEditor::CopyScratchToField() {
    copybackShader.DispatchSync(manager->device, manager->queue, uvec3(manager->uniformManager.parameters.textureResolution));
}

void MarchingCubes::FieldEditor::Destroy() {
    fieldTexture.Destroy();
    fieldScratchTexture.Destroy();
    fieldInitializer.Destroy();
    copybackShader.Destroy();
}