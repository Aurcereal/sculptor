@group(0) @binding(0) var outputTexture: texture_storage_3d<r32float,write>;

struct Parameters {
    texRes : u32,
    marchingCubesRes : u32,
    marchingCubesThreshold : f32
};
@group(0) @binding(1) var<uniform> u_Parameters : Parameters;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var p = (vec3f(id.xyz)/f32(u_Parameters.texRes))*2.-1.;
    var r = length(p)+sin(p.y*20.)*0.1;
    var amt = smoothstep(0.7, 0.6, r);
    textureStore(outputTexture, id, vec4f(amt, 0.1, 0.1, 1.0));
}