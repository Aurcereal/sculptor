@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var inputTextureSampler: sampler;
// unorm just means we manipulate using unsigned normalized in shader cuz it's a COLOR, when we transition to marching cubes we'll use float with no color meaning
@group(0) @binding(2) var outputTexture: texture_storage_3d<rgba8unorm,write>;

@compute 
@workgroup_size(4, 4, 4)
fn computeStuff(@builtin(global_invocation_id) id: vec3<u32>) {
    textureStore(outputTexture, id, vec4f(0.1, 0.9, 0.1, 1.0));
}