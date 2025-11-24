
@group(0) @binding(0) var<storage, read> countBuffers: array<atomic<u32>>; // (vertCount, indexCount)
@group(0) @binding(1) var<storage, read_write> indirectArgs: array<u32>; 

@compute 
@workgroup_size(4, 4, 4) 
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    if(id.x + id.y + id.z == 0u) {
        indirectArgs[0] = countBuffers[1];
    }
}