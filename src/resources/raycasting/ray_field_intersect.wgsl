@group(0) @binding(0) var fieldTexture: texture_3d<f32>;
@group(0) @binding(1) var fieldSampler: sampler;

struct RaycastInput {
    origin : vec3f,
    direction : vec3f
};
@group(0) @binding(2) var<uniform> raycastInput : RaycastInput;
@group(0) @binding(3) var<storage, read_write> intersectionBuffer: array<vec4f>; // (hitPos, norm)

struct Parameters {
    texRes : u32,
    marchingCubesRes : u32,
    marchingCubesThreshold : f32,
    bbxTRS : mat4x4f,
    bbxInvTRS : mat4x4f,
    bbxInverseTranspose : mat4x4f // Should be mat3x3 but alignment isn't working somehow
};
@group(0) @binding(4) var<uniform> u_Parameters : Parameters;

fn rayBoxIntersect(ro : vec3f, ird : vec3f) -> vec2f {
	let lb = vec3f(0.0);
	let rt = vec3f(1.0);

	let t1 = (lb.x - ro.x) * ird.x;
	let t2 = (rt.x - ro.x) * ird.x;
	let t3 = (lb.y - ro.y) * ird.y;
	let t4 = (rt.y - ro.y) * ird.y;
	let t5 = (lb.z - ro.z) * ird.z;
	let t6 = (rt.z - ro.z) * ird.z;

	let tMin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	let tMax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	return vec2f(tMin, tMax);
}

@compute 
@workgroup_size(4, 4, 4)
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    if(id.x + id.y + id.z == 0u) {
        //let stepSize = 0.04;

        let ro = raycastInput.origin;
        let rd = raycastInput.direction;

        let lro = (u_Parameters.bbxInvTRS * vec4f(ro, 1.0)).xyz;
        let lrd = (u_Parameters.bbxInvTRS * vec4f(rd, 0.0)).xyz;

        let ts = rayBoxIntersect(lro, 1.0/lrd);
        if(ts.x < ts.y) {
            let hitT = max(ts.x, 0.0); // For the inside box we'll start marching at ro
            var currT = hitT+0.001;

            let stepSize = (ts.y-ts.x)/100.0;
            for(var i: u32 = 0; i<100; i=i+1) {
                let currUV = lro + currT*lrd;
                let s = textureSampleLevel(fieldTexture, fieldSampler, currUV, 0.0).r;
                if(s >= u_Parameters.marchingCubesThreshold) {
                    let lHitPos = lro + lrd*currT;
                    // let lNorm = 

                    let hitPos = (u_Parameters.bbxTRS * vec4f(lHitPos, 1.0)).xyz;
                    let norm = vec3f(0.0,1.0,0.0);

                    intersectionBuffer[0] = vec4f(hitPos, 1.0);
                    intersectionBuffer[1] = vec4f(norm, 0.0);

                    return;
                }
                currT += stepSize;
            }

            // let lHitPos = lro + lrd*currT;
            // // let lNorm = 

            // let hitPos = (u_Parameters.bbxTRS * vec4f(lHitPos, 1.0)).xyz;
            // let norm = vec3f(0.0,1.0,0.0);

            // intersectionBuffer[0] = vec4f(hitPos, 1.0);
            // intersectionBuffer[1] = vec4f(norm, 0.0);
            
        }

        intersectionBuffer[0] = vec4f(0.0,0.0,0.0,0.0);
        intersectionBuffer[1] = vec4f(0.0,0.0,0.0,0.0);
    }
}