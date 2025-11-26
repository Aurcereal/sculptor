@group(0) @binding(0) var outputTexture: texture_storage_3d<r32float,write>;

struct Parameters {
    texRes : u32,
    marchingCubesRes : u32,
    marchingCubesThreshold : f32,
    flatShading : u32,
    bbxTRS : mat4x4f,
    bbxInvTRS : mat4x4f,
    bbxInverseTranspose : mat4x4f // Should be mat3x3 but alignment isn't working somehow
};
@group(0) @binding(1) var<uniform> u_Parameters : Parameters;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var p = (vec3f(id.xyz)/f32(u_Parameters.texRes))*2.-1.;
    var r = max(max(abs(p.x), abs(p.y)), abs(p.z));
    var amt = 0.4*smoothstep(0.8, 0.2, r);
    textureStore(outputTexture, id, vec4f(amt, 0.0, 0.0, 0.0));
}