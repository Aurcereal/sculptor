@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var inputColorTexture: texture_3d<f32>;
@group(0) @binding(2) var outputTexture: texture_storage_3d<r32float,write>;
@group(0) @binding(3) var outputColorTexture: texture_storage_3d<rgba8unorm,write>;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    textureStore(outputTexture, id, textureLoad(inputTexture, id, 0));
    textureStore(outputColorTexture, id, textureLoad(inputColorTexture, id, 0));
}