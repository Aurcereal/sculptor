@group(0) @binding(0) var outputTexture: texture_storage_3d<r32float,write>;
@group(0) @binding(1) var outputColorTexture: texture_storage_3d<rgba8unorm,write>;

struct Parameters {
    texRes : u32,
    marchingCubesRes : u32,
    marchingCubesThreshold : f32,
    flatShading : u32,
    bbxTRS : mat4x4f,
    bbxInvTRS : mat4x4f,
    bbxInverseTranspose : mat4x4f // Should be mat3x3 but alignment isn't working somehow
};
@group(0) @binding(2) var<uniform> u_Parameters : Parameters;

struct BrushParameters {
    brushType : u32,
    brushMult : f32,
    brushSize : f32,
    color : vec3f
};
@group(0) @binding(3) var<uniform> u_BrushParameters: BrushParameters;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var p = (vec3f(id.xyz)/f32(u_Parameters.texRes))*2.-1.;
    var r = length(p);//+sin(p.y*20.)*0.1;
    var amt = 0.4*smoothstep(0.8, 0.4, r); // Smooth field looks smooth, high gradients bad..
    textureStore(outputTexture, id, vec4f(amt, 0.0, 0.0, 0.0));
    textureStore(outputColorTexture, id, vec4f(step(u_Parameters.marchingCubesThreshold*0.7, amt) * u_BrushParameters.color*vec3(1.0,step(r, 0.5),step(r, 0.5)), 1.0));
}