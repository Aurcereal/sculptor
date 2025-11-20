@group(0) @binding(0) var outputTexture: texture_storage_3d<r32float,write>;

// TODO: Use UNIFORMS
const resolution: u32 = 256;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var p = (vec3f(id.xyz)/f32(resolution))*2.-1.;
    var r = length(p)+sin(p.y*20.)*0.1;
    var amt = smoothstep(0.8, 0.9, r);
    textureStore(outputTexture, id, vec4f(amt, 0.1, 0.1, 1.0));
}