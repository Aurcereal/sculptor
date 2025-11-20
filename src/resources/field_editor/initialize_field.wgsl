@group(0) @binding(0) var outputTexture: texture_storage_3d<rgba8unorm,write>;

@compute 
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var p = (vec3f(id.xyz)/256.)*2.-1.; // Use uniforms
    var r = length(p)+sin(p.y*20.)*0.1;
    var amt = smoothstep(0.8, 0.9, r);
    textureStore(outputTexture, id, vec4f(amt, 0.1, 0.1, 1.0));
}