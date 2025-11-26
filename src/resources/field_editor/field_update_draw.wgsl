@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var outputTexture: texture_storage_3d<r32float,write>;

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
@group(0) @binding(3) var<storage, read_write> intersectionBuffer: array<vec4f>; // (hitPos, norm)

struct BrushParameters {
    brushType : u32,
    brushMult : f32,
    brushSize : f32
};
@group(0) @binding(4) var<uniform> u_BrushParameters: BrushParameters;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var uv = (vec3f(id.xyz)/f32(u_Parameters.texRes));
    var p = (u_Parameters.bbxTRS * vec4f(uv, 1.0)).xyz;

    let pos = intersectionBuffer[0];
    if(pos.a > 0.1) {
        // TODO: uniform
        let brushPos = pos.xyz;
        let brushSize = u_BrushParameters.brushSize;
        let brushMult = u_BrushParameters.brushMult;

        //
        let diff = p - brushPos;
        var falloff = max(0., 1.-length(diff)/brushSize);//dot(diff, diff)/(brushSize*brushSize));
        falloff *= falloff*falloff*falloff;
        let amt = brushMult * falloff;

        let curr = textureLoad(inputTexture, id, 0).r;
        let newVal = clamp(curr+amt, 0.0, 1.0);
        textureStore(outputTexture, id, vec4(newVal,0.0,0.0,0.0));
    } else {
        textureStore(outputTexture, id, textureLoad(inputTexture, id, 0)); // TODO: Massively inefficient to recalculate everytime when it's just a double passthrough when we're not clicking
    }
}