@group(0) @binding(0) var inputTexture: texture_3d<f32>;
@group(0) @binding(1) var inputColorTexture: texture_3d<f32>;
@group(0) @binding(2) var fieldSampler: sampler;
@group(0) @binding(3) var fieldColorSampler: sampler;
@group(0) @binding(4) var outputTexture: texture_storage_3d<r32float,write>;
@group(0) @binding(5) var outputColorTexture: texture_storage_3d<rgba8unorm,write>;

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
@group(0) @binding(6) var<uniform> u_Parameters : Parameters;
@group(0) @binding(7) var<storage, read_write> intersectionBuffer: array<vec4f>; // (hitPos, norm)

struct BrushParameters {
    brushType : u32,
    brushMult : f32,
    brushSize : f32,
    brushHardness : f32,
    color : vec3f,
    drawShape : u32,
    paintTexture : u32,
    sculptTexture : u32,
    brushFollowNormal : u32
};
@group(0) @binding(8) var<uniform> u_BrushParameters: BrushParameters;


const BT_DRAW: u32 = 0;
const BT_TWIRL: u32 = 1;

const DS_SPHERE: u32 = 0;
const DS_CUBE: u32 = 1;
const DS_CONE: u32 = 2;
const DS_TRIANGLE: u32 = 3;
const DS_STAR: u32 = 4;

const PT_SOLIDCOLOR: u32 = 0;
const PT_SWIRLY: u32 = 1;
const PT_POLKADOT: u32 = 2;
const PT_STRIPES: u32 = 3;
const PT_CHECKER: u32 = 4;
const PT_CIRCEPATTERN: u32 = 5;
const PT_NOISY: u32 = 6;

const ST_NONE: u32 = 0;
const ST_CHECKER: u32 = 1;
const ST_POLKADOT: u32 = 2;
const ST_SPHEREPATTERN: u32 = 3;
const ST_GYROID: u32 = 4;
const ST_NOISY: u32 = 5;
const ST_COMB: u32 = 6;
const ST_CUBECOMB: u32 = 7;

fn fmod(x: f32, y: f32) -> f32 { return x - y * floor(x / y); }
fn vmod(x: vec3f, y: vec3f) -> vec3f { return x - y * floor(x / y); }

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

fn createFrame(fo: vec3f) -> mat3x3f {
    let up1 = vec3f(0.,1.,0.);
    let up2 = vec3f(0.,0.,1.);
    let ri1 = cross(up1, fo);
    let riFinal = normalize(step(dot(ri1,ri1), 0.)*cross(up2, fo) + ri1);
    let upFinal = normalize(cross(fo, riFinal));
    return mat3x3f(riFinal, upFinal, fo);
}

fn sdBox(p: vec3f, dim: vec3f) -> f32 {
    let lp = abs(p) - dim*.5;
    let ds = max(lp.x, max(lp.y, lp.z));
    let s = sqrt(max(lp.x, 0.)*lp.x + max(lp.y, 0.)*lp.y + max(lp.z, 0.)*lp.z);
    return step(s, 0.) * ds + s;
}

// const ST_POLKADOT: u32 = 2;
// const ST_SPHEREPATTERN: u32 = 3;
// const ST_GYROID: u32 = 4;

fn intersectSculptTexture(p: vec3f) -> f32 {
    switch u_BrushParameters.sculptTexture {
        case ST_NONE, ST_NOISY, default: {
            return 1.;
        }
        case ST_CHECKER: {
            let size = 0.2;
            let id = floor(p/size);
            let parity = step(abs(fmod(id.x+id.y+id.z, 2.)-1.), 0.5);
            return parity;
        }
        case ST_POLKADOT: {
            let size = 0.1;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            return u_Parameters.marchingCubesThreshold+(length(lp)-(.3*size));
        }
        case ST_SPHEREPATTERN: {
            let size = 0.5;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            let s1 = abs(length(lp)-size*.45)-0.1;
            let lp2 = vmod(lp, vec3f(size*.5))-vec3f(size*.25);
            let s2 = abs(length(lp2)-size*.225)-0.05;
            return -min(s1, s2);
        }
        case ST_GYROID: {
            return 0.;
        }
        case ST_CUBECOMB: {
            let size = 0.2;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            return sdBox(lp, vec3f(size*.9)); // u_Parameters.marchingCubesThreshold
        }
        case ST_COMB: {
            return step(sin(p.x*50.), 0.); // dot w (1,1,1)/sqrt(3)
        }
    }
}

@compute
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    let pos = intersectionBuffer[0];
    if(pos.a > 0.1) {
        var uv = (vec3f(id.xyz)/f32(u_Parameters.texRes));
        var mirrorAlignment = 1.;
        if(bool(u_Parameters.mirrorX)) {
            let mirrorDir = step(0.5, (u_Parameters.bbxInvTRS * pos).x)*2.-1.;
            uv.x = 0.5+mirrorDir*abs(uv.x-0.5);
            mirrorAlignment = mirrorDir*sign(uv.x-0.5);
        }
        var p = (u_Parameters.bbxTRS * vec4f(uv, 1.0)).xyz;

        let norm = intersectionBuffer[1].xyz;
        let brushPos = pos.xyz;
        let brushSize = u_BrushParameters.brushSize;
        let brushMult = u_BrushParameters.brushMult;

        // Calculate Falloff
        var lp = p - brushPos;
        if(bool(u_BrushParameters.brushFollowNormal)) {
            lp = transpose(createFrame(norm)) * lp;
            lp.z *= 2.5;
        }
        
        // Draw Shape
        var r = 0.; var falloff = 0.;
        switch u_BrushParameters.drawShape {
            case DS_SPHERE: {
                r = length(lp);
            }
            case DS_CUBE: {
                r = max(max(abs(lp.x), abs(lp.y)), abs(lp.z));
            }
            default: {}
        }

        // TODO: Mess with r for like noisy sculpt texture

        falloff = max(0., 1.-smoothstep(u_BrushParameters.brushHardness, 1.0, r/brushSize));
        falloff *= falloff*falloff*falloff;
        let amt = brushMult * falloff;

        let sculptAmt = intersectSculptTexture(p) * (1.-f32(u_Parameters.paintMode)) * amt;
        var newSculptVal = 0.;
        var newCol = vec4f(0.);
        switch u_BrushParameters.brushType {
            case BT_DRAW: {
                let curr = textureLoad(inputTexture, id, 0).r;
                newSculptVal = clamp(curr+sculptAmt, -0.5, borderFalloff(uv));

                // Paint depending on Paint Texture
                // TODO: Mess with colAmt (and the target color) for Paint Texture
                let colAmt = 10. * max(0., amt);
                let currCol = textureLoad(inputColorTexture, id, 0);
                newCol = mix(currCol, vec4f(u_BrushParameters.color, 1.0), colAmt);
            }
            case BT_TWIRL: {
                let rp = brushPos + (rot(norm, 2.5*sculptAmt) * vec4f(p-brushPos, 1.)).xyz;
                var ruv = (u_Parameters.bbxInvTRS * vec4f(rp, 1.)).xyz;
                ruv.x = 0.5+mirrorAlignment*(ruv.x-0.5);
                newSculptVal = textureSampleLevel(inputTexture, fieldSampler, ruv+vec3f(0.5)/f32(u_Parameters.texRes), 0.).r; 

                let rpCol = brushPos + (rot(norm, 2.5*amt) * vec4f(p-brushPos, 1.)).xyz;
                var ruvCol = (u_Parameters.bbxInvTRS * vec4f(rpCol, 1.)).xyz;
                ruvCol.x = 0.5+mirrorAlignment*(ruvCol.x-0.5);
                newCol = textureSampleLevel(inputColorTexture, fieldColorSampler, ruvCol+vec3f(0.5)/f32(u_Parameters.texRes), 0.);
            }
            default: {}
        }
        textureStore(outputTexture, id, vec4(newSculptVal,0.0,0.0,0.0));
        textureStore(outputColorTexture, id, newCol);
        
        
    } else {
        // Massively inefficient passthrough (why passthrough at all when not clicking?)
        textureStore(outputTexture, id, textureLoad(inputTexture, id, 0));
        textureStore(outputColorTexture, id, textureLoad(inputColorTexture, id, 0));
    }
}