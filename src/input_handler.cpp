#include "marching_cubes_manager.h"

void MarchingCubes::InputHandler::OnLMouseClick(vec2 pos) {
    if(manager->intersectionTextureGeneratedOnlyOnClick) {
        manager->fieldEditor.CopyToIntersectionTexture();
    }

    mouseDown = true;
    vec3 rd = manager->camera.Raycast(pos);

    vec3 hitPos; mat3x3 frame;
    manager->raycaster.RayFieldIntersect(manager->camera.pos, rd, &hitPos, &frame, true);

    manager->cursorOperator.OnClick(hitPos, frame);
}
void MarchingCubes::InputHandler::OnLMouseRelease(vec2) {
    mouseDown = false;
    manager->raycaster.ResetIntersectionBuffer();
}
void MarchingCubes::InputHandler::OnMouseMove(vec2, vec2) {
    
}
void MarchingCubes::InputHandler::HandleUpdates() {
    // Handle Subtraction w/ Alt key
    bool currentlyAdding = manager->guiToParams.brushParameters.brushMult > 0.0f;
    bool willBeAdding = !manager->inputManager->altDown;
    if(willBeAdding != currentlyAdding) {
        manager->guiToParams.brushParameters.brushMult = (willBeAdding ? 1.0f : -1.0f) * abs(manager->guiToParams.brushParameters.brushMult);
        manager->uniformManager.UpdateBrushParameters();
    }
}