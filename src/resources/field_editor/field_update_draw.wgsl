@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var inputColorTexture: texture_3d<f32>;
@group(0) @binding(2) var fieldSampler: sampler;
@group(0) @binding(3) var outputTexture: texture_storage_3d<r32float,write>;
@group(0) @binding(4) var outputColorTexture: texture_storage_3d<rgba8unorm,write>;

struct Parameters {
    texRes : u32,
    marchingCubesRes : u32,
    marchingCubesThreshold : f32,
    flatShading : u32,
    bbxTRS : mat4x4f,
    bbxInvTRS : mat4x4f,
    bbxInverseTranspose : mat4x4f, // Should be mat3x3 but alignment isn't working somehow
    mirrorX : u32,
    paintMode : u32
};
@group(0) @binding(5) var<uniform> u_Parameters : Parameters;
@group(0) @binding(6) var<storage, read_write> intersectionBuffer: array<vec4f>; // (hitPos, norm)

struct BrushParameters {
    brushShape : u32,
    brushMult : f32,
    brushSize : f32,
    brushHardness : f32,
    color : vec3f,
    brushOperation : u32
};
@group(0) @binding(7) var<uniform> u_BrushParameters: BrushParameters;

fn borderFalloff(uv: vec3f) -> f32 {
    let ds = vec3f(0.5) - abs(uv-vec3f(0.5));
    let d = min(min(ds.x, ds.y), ds.z);
    return smoothstep(0.01, 0.1, d);
}

fn rot(v: vec3f, o: f32) -> mat4x4f {
    // https://en.wikipedia.org/wiki/Rotation_matrix
    let cosAngle = cos(o);
    let sinAngle = sin(o);
    let oneMinusCosAngle = 1.-cosAngle;
    return mat4x4f(
        vec4f(v[0]*v[0]*oneMinusCosAngle+cosAngle, v[0]*v[1]*oneMinusCosAngle+v[2]*sinAngle, v[0]*v[2]*oneMinusCosAngle-v[1]*sinAngle, 0.),
        vec4f(v[0]*v[1]*oneMinusCosAngle-v[2]*sinAngle, v[1]*v[1]*oneMinusCosAngle+cosAngle, v[1]*v[2]*oneMinusCosAngle+v[0]*sinAngle, 0.),
        vec4f(v[0]*v[2]*oneMinusCosAngle+v[1]*sinAngle, v[1]*v[2]*oneMinusCosAngle-v[0]*sinAngle, v[2]*v[2]*oneMinusCosAngle+cosAngle, 0.),
        vec4f(0., 0., 0., 1.)
        );
}

const BSHAPE_SPHERE: u32 = 0;
const BSHAPE_BOX: u32 = 1;
const BSHAPE_TRI: u32 = 2;
const BSHAPE_CONE: u32 = 3;

const BOP_DRAW: u32 = 0;
const BOP_TWIRL: u32 = 1;

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    let pos = intersectionBuffer[0];
    if(pos.a > 0.1) {
        var uv = (vec3f(id.xyz)/f32(u_Parameters.texRes));
        if(bool(u_Parameters.mirrorX)) {
            let mirrorDir = step(0.5, (u_Parameters.bbxInvTRS * pos).x)*2.-1.;
            uv.x = 0.5+mirrorDir*abs(uv.x-0.5);
        }
        var p = (u_Parameters.bbxTRS * vec4f(uv, 1.0)).xyz;

        let norm = intersectionBuffer[1].xyz;
        let brushPos = pos.xyz;
        let brushSize = u_BrushParameters.brushSize;
        let brushMult = u_BrushParameters.brushMult;

        //
        var diff = p - brushPos;
        diff = 2.5*norm*dot(diff,norm) + (diff - norm*dot(diff, norm));
        let r = length(diff);
        var falloff = max(0., 1.-smoothstep(u_BrushParameters.brushHardness, 1.0, r/brushSize));
        falloff *= falloff*falloff*falloff;
        let amt = brushMult * falloff;

        let sculptAmt = (1.-f32(u_Parameters.paintMode)) * amt;
        let colAmt = 10. * amt;

        switch(u_Parameters.brushType) {
            case 
        }
        let curr = textureLoad(inputTexture, id, 0).r;
        let rp = brushPos + (rot(norm, 5.*sculptAmt) * vec4f(p-brushPos, 1.)).xyz;
        var ruv = (u_Parameters.bbxInvTRS * vec4f(rp, 1.)).xyz;
        var newVal = textureSampleLevel(inputTexture, fieldSampler, ruv+vec3f(0.5)/f32(u_Parameters.texRes), 0.).r; 
        //var newVal = clamp(curr+sculptAmt, 0.0, borderFalloff(uv));
        textureStore(outputTexture, id, vec4(newVal,0.0,0.0,0.0));

        let currCol = textureLoad(inputColorTexture, id, 0);
        let newCol = mix(currCol, vec4f(u_BrushParameters.color, 1.0), colAmt);
        textureStore(outputColorTexture, id, newCol);
    } else {
        textureStore(outputTexture, id, textureLoad(inputTexture, id, 0)); // TODO: Massively inefficient to recalculate everytime when it's just a double passthrough when we're not clicking
        textureStore(outputColorTexture, id, textureLoad(inputColorTexture, id, 0));
    }
}