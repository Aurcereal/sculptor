
#include "marching_cubes_manager.h"

using namespace std;
namespace SP = ShaderParameter;

void MarchingCubes::FieldEditor::Initialize() {
    assert(manager->guiToParams.initialized && manager->uniformManager.initialized && manager->raycaster.initialized);

    fieldTexture.Initialize(manager->device, uvec3(manager->guiToParams.parameters.textureResolution), TextureFormat::R32Float, true, true);
    fieldScratchTexture.Initialize(manager->device, uvec3(manager->guiToParams.parameters.textureResolution), TextureFormat::R32Float, true, true);

    fieldColorTexture.Initialize(manager->device, uvec3(manager->guiToParams.parameters.textureResolution), TextureFormat::RGBA8Unorm, true, true);
    fieldColorScratchTexture.Initialize(manager->device, uvec3(manager->guiToParams.parameters.textureResolution), TextureFormat::RGBA8Unorm, true, true);

    //
    vector<ShaderParameter::Parameter> params = {
        SP::Parameter(SP::UTexture{&fieldTexture, true, true}),
        SP::Parameter(SP::UTexture{&fieldColorTexture, true, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.brushParameterBuffer})
    };
    sphereFieldInitializer.Initialize(manager->device, params, "./field_editor/sphere_initialize_field.wgsl");
    cubeFieldInitializer.Initialize(manager->device, params, "./field_editor/plane_initialize_field.wgsl");

    //
    vector<ShaderParameter::Parameter> copybackParams = {
        SP::Parameter(SP::UTexture{&fieldScratchTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldColorScratchTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldTexture, true, true}),
        SP::Parameter(SP::UTexture{&fieldColorTexture, true, true})
    };
    copybackShader.Initialize(manager->device, copybackParams, "./field_editor/copy_back.wgsl");
    
    //
    vector<ShaderParameter::Parameter> drawUpdateParams = {
        SP::Parameter(SP::UTexture{&fieldTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldColorTexture, true, false}),
        SP::Parameter(SP::UTexture{&fieldScratchTexture, true, true}),
        SP::Parameter(SP::UTexture{&fieldColorScratchTexture, true, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.parameterBuffer}),
        SP::Parameter(SP::UBuffer{&manager->raycaster.intersectionBuffer, true}),
        SP::Parameter(SP::UUniform{&manager->uniformManager.brushParameterBuffer}),
    };
    fieldDrawUpdater.Initialize(manager->device, drawUpdateParams, "./field_editor/field_update_draw.wgsl");

    std::cout << "Initialized Field Editor" << std::endl;
    initialized = true;
}

void MarchingCubes::FieldEditor::GenerateSphereField() {
    sphereFieldInitializer.DispatchSync(manager->device, manager->queue, uvec3(manager->guiToParams.parameters.textureResolution));
}
void MarchingCubes::FieldEditor::GenerateCubeField() {
    cubeFieldInitializer.DispatchSync(manager->device, manager->queue, uvec3(manager->guiToParams.parameters.textureResolution));
}

void MarchingCubes::FieldEditor::UpdateField() {
    fieldDrawUpdater.DispatchSync(manager->device, manager->queue, uvec3(manager->guiToParams.parameters.textureResolution));
    CopyScratchToField();
}

void MarchingCubes::FieldEditor::CopyScratchToField() {
    copybackShader.DispatchSync(manager->device, manager->queue, uvec3(manager->guiToParams.parameters.textureResolution));
}

void MarchingCubes::FieldEditor::Destroy() {
    fieldTexture.Destroy();
    fieldScratchTexture.Destroy();
    sphereFieldInitializer.Destroy();
    copybackShader.Destroy();
}