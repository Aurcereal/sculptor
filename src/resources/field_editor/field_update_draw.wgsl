@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var outputTexture: texture_storage_3d<r32float,write>;

struct CameraTimeParameters {
    projectionMatrix : mat4x4f,
    viewMatrix : mat4x4f,
    modelMatrix : mat4x4f,
    time : f32
};
@group(0) @binding(2) var<uniform> u_CameraTimeParameters: CameraTimeParameters;

// TODO: Use UNIFORMS
const resolution: u32 = 256;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var p = (vec3f(id.xyz)/f32(resolution))*2.-1.;

    // TODO: uniform
    let brushPos = vec3f(0., sin(u_CameraTimeParameters.time*8.0)*0.6, 0.);
    let brushSize = 0.2; // TODO: uniform should be in world space, convert it to uv space
    let brushMult = 0.3;

    //
    let diff = p - brushPos;
    let falloff = max(0., 1.-length(diff)/brushSize);//dot(diff, diff)/(brushSize*brushSize));
    let amt = brushMult * falloff;

    let curr = textureLoad(inputTexture, id, 0);
    textureStore(outputTexture, id, curr+vec4f(amt, 0., 0., 0.)); // TODO: this should just be float
}