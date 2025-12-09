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
    paintMode : u32,
    lightDirection : vec3f
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

fn hash31(p3: vec3f) -> f32 // From https://www.shadertoy.com/view/4djSRW
{
	var p3v  = fract(p3+vec3f(100.) * .1031);
    p3v += dot(p3, p3v.zyx + 31.32);
    return fract((p3v.x + p3v.y) * p3v.z);
}

fn hash33(p3i: vec3f) -> vec3f {
    var p3 = p3i;
	p3 = fract(p3 * vec3f(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy + p3.yxx)*p3.zyx);
}

fn rotZ(o: f32) -> mat3x3f {
    return mat3x3f(cos(o), sin(o), 0., -sin(o), cos(o), 0., 0., 0., 1.);
}

fn rotX(o: f32) -> mat3x3f {
    return mat3x3f(1., 0., 0., 0., cos(o), sin(o), 0., -sin(o), cos(o));
}

fn voronoi(p: vec3f) -> f32 {
    let id = floor(p);
    let lp = p-id;

    var minDist = 1000.;
    for(var x: i32 = -1; x<=1; x=x+1) {
        for(var y: i32 = -1; y<=1; y=y+1) {
            for(var z: i32 = -1; z<=1; z=z+1) {
                let currCell = id+vec3f(vec3i(x,y,z));
                let randVal = hash33(currCell);

                let orbPos = currCell+randVal;
                let dist = length(p-orbPos);
                minDist = min(minDist, dist);
            }
        }
    }

    return minDist;
}

// START https://gist.github.com/munrocket/236ed5ba7e409b8bdf1ff6eca5dcdc39
fn mod289(x: vec4f) -> vec4f { return x - floor(x * (1. / 289.)) * 289.; }
fn perm4(x: vec4f) -> vec4f { return mod289(((x * 34.) + 1.) * x); }
fn noise(p: vec3f) -> f32 {
    let a = floor(p);
    var d: vec3f = p - a;
    d = d * d * (3. - 2. * d);

    let b = a.xxyy + vec4f(0., 1., 0., 1.);
    let k1 = perm4(b.xyxy);
    let k2 = perm4(k1.xyxy + b.zzww);

    let c = k2 + a.zzzz;
    let k3 = perm4(c);
    let k4 = perm4(c + 1.);

    let o1 = fract(k3 * (1. / 41.));
    let o2 = fract(k4 * (1. / 41.));

    let o3 = o2 * d.z + o1 * (1. - d.z);
    let o4 = o3.yw * d.x + o3.xz * (1. - d.x);

    return o4.y * d.y + o4.x * (1. - d.y);
}
// END

// START SDFs taken From IQ
fn sdTriangle(pi: vec2f, r: f32) -> f32
{
    var p = pi;
    let k = sqrt(3.0);
    p.x = abs(p.x) - r;
    p.y = p.y + r/k;
    if( p.x+k*p.y>0.0 ) { p = vec2f(p.x-k*p.y,-k*p.x-p.y)/2.0; }
    p.x -= clamp( p.x, -2.0*r, 0.0 );
    return -length(p)*sign(p.y);
}
fn sdStar(pi: vec2f, r: f32) -> f32
{
    let k1x = 0.809016994; // cos(π/ 5) = ¼(√5+1)
    let k2x = 0.309016994; // sin(π/10) = ¼(√5-1)
    let k1y = 0.587785252; // sin(π/ 5) = ¼√(10-2√5)
    let k2y = 0.951056516; // cos(π/10) = ¼√(10+2√5)
    let k1z = 0.726542528; // tan(π/ 5) = √(5-2√5)
    let v1  = vec2f( k1x,-k1y);
    let v2  = vec2f(-k1x,-k1y);
    let v3  = vec2f( k2x,-k2y);
    
    var p = pi;
    p.x = abs(p.x);
    p -= 2.0*max(dot(v1,p),0.0)*v1;
    p -= 2.0*max(dot(v2,p),0.0)*v2;
    p.x = abs(p.x);
    p.y -= r;
    return length(p-v3*clamp(dot(p,v3),0.0,k1z*r))
           * sign(p.y*v3.x-p.x*v3.y);
}
fn sdCone(p: vec3f, dim: vec2f) -> f32
{
    let q = vec2f(dim.x, -dim.y);
        
    var w = vec2f( length(p.xz), p.y );
    w.y -= dim.y*.5;
    let a = w - q*clamp( dot(w,q)/dot(q,q), 0.0, 1.0 );
    let b = w - q*vec2f( clamp( w.x/q.x, 0.0, 1.0 ), 1.0 );
    let k = sign( q.y );
    let d = min(dot( a, a ),dot(b, b));
    let s = max( k*(w.x*q.y-w.y*q.x),k*(w.y-q.y)  );
    return sqrt(d)*sign(s);
}
// END SDFs taken from IQ

fn fbm(p: vec3f) -> f32 {
    let iterations = 4u;
    let rot = rotX(32.53) * rotZ(18.4) * rotX(41.2);

    let scaleMult = 2.;
    let decay = .5;
    
    var sum = 0.;
    var currMult = .5;

    var q = 5.*p;
    for(var i: u32=0; i<iterations; i=i+1) {
        sum += noise(q)*currMult;

        q *= scaleMult;
        q += vec3f(13.513,591.,219.);
        q = rot * q;
        currMult *= decay;
    }

    return sum;
}

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
const PT_CIRCLEPATTERN: u32 = 5;
const PT_NOISY: u32 = 6;

const ST_NONE: u32 = 0;
const ST_CHECKER: u32 = 1;
const ST_POLKADOT: u32 = 2;
const ST_SPHEREPATTERN: u32 = 3;
const ST_GYROID: u32 = 4;
const ST_NOISY: u32 = 5;
const ST_COMB: u32 = 6;
const ST_CUBECOMB: u32 = 7;
const ST_BUMPY: u32 = 8;
const ST_VORONOI: u32 = 9;

const PI: f32 = 3.141592;

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

fn rot2D(o: f32) -> mat2x2f {
    return mat2x2f(cos(o), sin(o), -sin(o), cos(o));
}

fn createFrame(fo: vec3f) -> mat3x3f {
    let up1 = vec3f(0.,1.,0.);
    let up2 = vec3f(0.,0.,1.);
    let ri1 = cross(up1, fo);
    let riFinal = normalize(step(dot(ri1,ri1), 0.)*cross(up2, fo) + ri1);
    let upFinal = normalize(cross(fo, riFinal));
    return mat3x3f(riFinal, upFinal, fo);
}

fn toSpherical(p: vec3f) -> vec3f {
    let r = length(p);
    return vec3f(r, atan2(p.z, p.x), asin(p.y/r));
}

fn sdBox(p: vec3f, dim: vec3f) -> f32 {
    let lp = abs(p) - dim*.5;
    let ds = max(lp.x, max(lp.y, lp.z));
    let s = sqrt(max(lp.x, 0.)*lp.x + max(lp.y, 0.)*lp.y + max(lp.z, 0.)*lp.z);
    return step(s, 0.) * ds + s;
}

fn intersectSculptTexture(p: vec3f) -> f32 {
    switch u_BrushParameters.sculptTexture {
        case ST_NONE, ST_NOISY, default: {
            return 1.;
        }
        case ST_CHECKER: {
            let sp = intersectionBuffer[4].xy;
            let brushPos = intersectionBuffer[0].xyz;
            let normFrame = mat3x3f(intersectionBuffer[2].xyz, intersectionBuffer[3].xyz, intersectionBuffer[1].xyz);
            var lp = transpose(normFrame) * (p-brushPos);
            lp += vec3f(sp,0.);

            // lp *= vec3f(10.,10.,1.);
            // let h = sin(lp.x)+cos(lp.y);
            // let amt = h-lp.z;
            let size = 0.14;
            //lp = vec3f(rot2D(lp.z*3.5) * lp.xy, lp.z);
            let lmp = vec3f(vec2f(lp.xy-floor(lp.xy/size)*size-size*.5), lp.z);
            let dCone = sdCone(lmp.xzy*vec3f(1.,0.1,1.), vec2f(0.08, 0.1*3.));
            let amt = u_Parameters.marchingCubesThreshold-5.*5.*dCone;
            return amt;
            // let size = 0.2;
            // let id = floor(p/size);
            // let parity = step(abs(fmod(id.x+id.y+id.z, 2.)-1.), 0.5);
            // return parity;
        }
        case ST_POLKADOT: {
            let size = 0.25;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            return u_Parameters.marchingCubesThreshold-2.*(length(lp)-(.1*size));
        }
        case ST_SPHEREPATTERN: {
            let size = 0.5;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            let s1 = abs(length(lp)-size*.45)-0.1;
            let lp2 = vmod(lp, vec3f(size*.5))-vec3f(size*.25);
            let s2 = abs(length(lp2)-size*.225)-0.05;
            return -min(s1, s2); // Could scalar this
        }
        case ST_GYROID: { // mini.gmshaders.com/p/dot-noise
            //The golden ratio: mini.gmshaders.com/p/phi
            let phi = 1.618033988;
            let scale = 10.;
            let gold = mat3x3f(
                -0.571464913, 0.814921382, 0.096597072,
                -0.278044873, -0.303026659, 0.911518454,
                0.772087367, 0.494042493, 0.399753815);
            return u_Parameters.marchingCubesThreshold+(dot(cos(scale * gold * p), sin(phi * scale * p * gold))+1.5)*.2;
        }
        case ST_CUBECOMB: {
            let size = 0.2;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            return sdBox(lp, vec3f(size*.9)); // u_Parameters.marchingCubesThreshold
        }
        case ST_COMB: {
            return step(sin(dot(p,vec3f(1.)/sqrt(3.))*50.), 0.);
        }
        case ST_BUMPY: {
            let size = 0.1;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            return (length(lp)-(.3*size));
        }
        case ST_VORONOI: {
            return voronoi(p*3.)*1.25*1.5-1. - 1.5-.5;
        }
    }
}

fn getPaintColor(p: vec3f, lp2d: vec2f, currAmt: f32) -> vec4f {
    switch u_BrushParameters.paintTexture {
        case default, PT_SOLIDCOLOR: {
            return vec4f(u_BrushParameters.color, currAmt);
        }
        case PT_SWIRLY: {
            var sph = toSpherical(p);
            let repSize = 0.12;
            sph.x += 3.*repSize*(sph.y+sph.z)/PI;
            let stripeFac = 0.5;
            let lr = abs(fmod(sph.x, repSize)-repSize*.5);
            let dist = lr-repSize*.5*stripeFac;
            let exists = step(dist, 0.);
            if(bool(u_Parameters.paintMode)) { return vec4f(u_BrushParameters.color, exists*currAmt); }
            else { return vec4f(u_BrushParameters.color+(1.-exists)*(vec3f(1.)-2.*u_BrushParameters.color), currAmt); }
        }
        case PT_POLKADOT: {
            let size = 0.07;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            let exists = step(length(lp), .3*size);
            if(bool(u_Parameters.paintMode)) { return vec4f(u_BrushParameters.color, exists*currAmt); }
            else { return vec4f(u_BrushParameters.color+(1.-exists)*(vec3f(1.)-2.*u_BrushParameters.color), currAmt); }
        }
        case PT_STRIPES: {
            let repSize = 0.2;
            let stripeFac = 0.5;
            let l = dot(p, vec3f(1.f)/sqrt(3.));
            let ll = fmod(l, repSize)/repSize-.5;
            let exists = step(abs(ll), stripeFac*.5);
            if(bool(u_Parameters.paintMode)) { return vec4f(u_BrushParameters.color, exists*currAmt); }
            else { return vec4f(u_BrushParameters.color+(1.-exists)*(vec3f(1.)-2.*u_BrushParameters.color), currAmt); }
        }
        case PT_CIRCLEPATTERN: {
            let size = 0.5;
            let lp = vmod(p, vec3f(size))-vec3f(size*.5);
            let s1 = abs(length(lp)-size*.55)-0.04;
            let lp2 = vmod(lp, vec3f(size*.5))-vec3f(size*.25);
            let s2 = abs(length(lp2)-size*.265)-0.02;
            let lp3 = vmod(lp2, vec3f(size*.25))-vec3f(size*.125);
            let s3 = abs(length(lp3)-size*.11)-0.008;
            let exists = step(min(s1, min(s2, s3)), 0.);
            if(bool(u_Parameters.paintMode)) { return vec4f(u_BrushParameters.color, exists*currAmt); }
            else { return vec4f(u_BrushParameters.color+(1.-exists)*(vec3f(1.)-2.*u_BrushParameters.color), currAmt); }
        }
        case PT_NOISY: {
            var val = step(hash31(p*10.), .5);//step(abs(fbm(p*2.)-.5),0.15);
            if(bool(u_Parameters.paintMode)) { return vec4f(u_BrushParameters.color, val*currAmt); }
            else { return vec4f(u_BrushParameters.color+(1.-val)*(vec3f(1.)-2.*u_BrushParameters.color), currAmt); }
        }
        case PT_CHECKER: {
            //let sp = 20.*(intersectionBuffer[4].xy+lp2d);
            let size = 0.2;//*40.;
            let id = floor(p/size);
            //let exists = step(abs(fmod(id.x+id.y, 2.)-1.), 0.5);
            let exists = step(abs(fmod(id.x+id.y+id.z, 2.)-1.), 0.5);
            if(bool(u_Parameters.paintMode)) { return vec4f(u_BrushParameters.color, exists*currAmt); }
            else { return vec4f(u_BrushParameters.color+(1.-exists)*(vec3f(1.)-2.*u_BrushParameters.color), currAmt); }
        }
    }
}

fn sdExtrusion(p: vec3f, dist2D: f32, size: f32) -> f32 {
    let zDist = abs(p.z)-size*.5;
    let ds = max(zDist, dist2D);
    let s = sqrt(max(zDist, 0.)*zDist + max(dist2D, 0.)*dist2D);
    return step(s, 0.) * ds + s;
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
            mirrorAlignment = mirrorDir*sign(uv.x-0.5);
            uv.x = 0.5+mirrorDir*abs(uv.x-0.5);
        }
        var p = (u_Parameters.bbxTRS * vec4f(uv, 1.0)).xyz;

        let norm = intersectionBuffer[1].xyz;
        let normFrame = mat3x3f(intersectionBuffer[2].xyz, intersectionBuffer[3].xyz, intersectionBuffer[1].xyz);
        let brushPos = pos.xyz;
        let brushSize = u_BrushParameters.brushSize;
        let brushMult = u_BrushParameters.brushMult;

        // Calculate Falloff
        var lp = p - brushPos;
        let lp2d = (transpose(normFrame) * lp).xy;
        if(bool(u_BrushParameters.brushFollowNormal)) {

            lp = transpose(normFrame) * lp; //createFrame(norm)
            lp.z *= 2.5;
        }
        
        // Draw Shape
        var r = 0.; if(u_BrushParameters.sculptTexture == ST_NOISY) { r -= .2*fbm(p*.9); }
        var falloff = 0.;
        switch u_BrushParameters.drawShape {
            case DS_SPHERE: {
                r += length(lp);
                falloff = max(0., 1.-smoothstep(u_BrushParameters.brushHardness+.99*0., 1.0, r/brushSize)); // .99 on spikes, or can make falloff for things like grass have r be xy in local
            }
            case DS_CUBE: {
                r += max(max(abs(lp.x), abs(lp.y)), abs(lp.z));
                falloff = max(0., 1.-smoothstep(u_BrushParameters.brushHardness, 1.0, r/brushSize));
            }
            case DS_CONE: {
                r += -(u_Parameters.marchingCubesThreshold-4.*sdCone(lp.xzy, brushSize*vec2f(0.5, 1.))); // min(1., .. w/2. on field scale)
                falloff = (1.-smoothstep(u_BrushParameters.brushHardness, 1.0, r)); // just max(0,)
            }
            case DS_TRIANGLE: {
                r += 4.*sdExtrusion(lp, sdTriangle(lp.xy, .5*brushSize), .5*brushSize)-u_Parameters.marchingCubesThreshold;
                falloff = (1.-smoothstep(u_BrushParameters.brushHardness, 1.0, r)); 
            }
            case DS_STAR: {
                r += 4.*sdExtrusion(lp, sdStar(lp.xy, .5*brushSize), .5*brushSize)-u_Parameters.marchingCubesThreshold;
                falloff = (1.-smoothstep(u_BrushParameters.brushHardness, 1.0, r)); 
            }
            default: {}
        }

        falloff *= falloff*falloff*falloff;
        let amt = brushMult * falloff;

        let sculptAmt = intersectSculptTexture(p) * (1.-f32(u_Parameters.paintMode)) * amt;
        var newSculptVal = 0.;
        var newCol = vec4f(0.);
        switch u_BrushParameters.brushType {
            case BT_DRAW: {
                let curr = textureLoad(inputTexture, id, 0).r;
                newSculptVal = clamp(curr+sculptAmt, -0.5, borderFalloff(uv));

                let colAmt = 10. * max(0., amt);
                let colChangeInfo = getPaintColor(p, lp2d, colAmt);
                let currCol = textureLoad(inputColorTexture, id, 0);
                newCol = mix(currCol, vec4f(colChangeInfo.rgb, 1.0), colChangeInfo.a);
            }
            case BT_TWIRL: {
                let rp = brushPos + (rot(norm, 2.5*sculptAmt) * vec4f(p-brushPos, 1.)).xyz;
                var ruv = (u_Parameters.bbxInvTRS * vec4f(rp, 1.)).xyz;
                ruv.x = 0.5+mirrorAlignment*(ruv.x-0.5);
                newSculptVal = borderFalloff(ruv) * textureSampleLevel(inputTexture, fieldSampler, ruv+vec3f(0.5)/f32(u_Parameters.texRes), 0.).r; 

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