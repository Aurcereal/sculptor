@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var outputTexture: texture_storage_3d<r32float,write>;

struct Parameters {
    texRes : u32,
    marchingCubesRes : u32,
    marchingCubesThreshold : f32,
    bbxTRS : mat4x4f,
    bbxInvTRS : mat4x4f,
    bbxInverseTranspose : mat4x4f
};
@group(0) @binding(2) var<uniform> u_Parameters : Parameters;
@group(0) @binding(3) var<storage, read_write> intersectionBuffer: array<vec4f>; // (hitPos, norm)

struct CameraTimeParameters {
    projectionMatrix : mat4x4f,
    viewMatrix : mat4x4f,
    modelMatrix : mat4x4f,
    time : f32
};
@group(0) @binding(4) var<uniform> u_CameraTimeParameters: CameraTimeParameters; // Probably only need this uniform temporarily

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var uv = (vec3f(id.xyz)/f32(u_Parameters.texRes));
    var p = (u_Parameters.bbxTRS * vec4f(uv, 1.0)).xyz;

    let pos = intersectionBuffer[0];
    if(pos.a > 0.1) {
        // TODO: uniform
        let brushPos = vec3f(0., sin(u_CameraTimeParameters.time*8.0)*0.6, 0.9);
        let brushSize = 0.9;
        let brushMult = -0.3*10.;

        //
        let diff = p - brushPos;
        let falloff = max(0., 1.-length(diff)/brushSize);//dot(diff, diff)/(brushSize*brushSize));
        let amt = brushMult * falloff;

        let curr = textureLoad(inputTexture, id, 0);
        textureStore(outputTexture, id, curr+vec4f(amt, 0., 0., 0.)); // TODO: this should just be float
    } else {
        textureStore(outputTexture, id, textureLoad(inputTexture, id, 0)); // TODO: Massively inefficient to recalculate everytime when it's just a double passthrough when we're not clicking
    }
}