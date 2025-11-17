@group(0) @binding(0) var<storage, read_write> outputVertices: array<f32>; // Try vec3 or smth MAYBE?
@group(0) @binding(1) var<storage, read_write> outputIndices: array<u32>; 
@group(0) @binding(2) var<storage, read_write> countBuffers: array<atomic<u32>>; // (vertCount, indexCount)

@compute 
@workgroup_size(4, 4, 4) // TODO: make 3D
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var i = id.x;
    if(id.y + id.z != 0u) { return; }
    if(i == 1u) {
        let p1 = vec3f(-1.0, -1.0, 0.0);
        let p2 = vec3f(1.0, -1.0, 0.0);
        let p3 = vec3f(-1.0, 1.0, 0.0);
        var vertIndex = 0u;//atomicAdd(&countBuffers[0], 9u);

        outputVertices[vertIndex + 0] = p1.x;
        outputVertices[vertIndex + 1] = p1.y;
        outputVertices[vertIndex + 2] = p1.z;
        outputVertices[vertIndex + 3] = 0.;
        outputVertices[vertIndex + 4] = 0.;
        outputVertices[vertIndex + 5] = 0.;
        outputVertices[vertIndex + 6] = p2.x;
        outputVertices[vertIndex + 7] = p2.y;
        outputVertices[vertIndex + 8] = p2.z;
        outputVertices[vertIndex + 9] = 0.;
        outputVertices[vertIndex + 10] = 0.;
        outputVertices[vertIndex + 11] = 0.;
        outputVertices[vertIndex + 12] = p3.x;
        outputVertices[vertIndex + 13] = p3.y;
        outputVertices[vertIndex + 14] = p3.z;
        outputVertices[vertIndex + 15] = 0.;
        outputVertices[vertIndex + 16] = 0.;
        outputVertices[vertIndex + 17] = 0.;

        var triIndex = 0u;//atomicAdd(&countBuffers[1], 3u);
        outputIndices[triIndex + 0] = (vertIndex/6u) + 0;
        outputIndices[triIndex + 1] = (vertIndex/6u) + 1;
        outputIndices[triIndex + 2] = (vertIndex/6u) + 2;
    } else if(i==0u) {
        let p1 = vec3f(1.0, 1.0, 0.0);
        let p2 = vec3f(1.0, -1.0, 0.0);
        let p3 = vec3f(-1.0, 1.0, 0.0);
        var vertIndex = 18u;//atomicAdd(&countBuffers[0], 9u);

        outputVertices[vertIndex + 0] = p1.x;
        outputVertices[vertIndex + 1] = p1.y;
        outputVertices[vertIndex + 2] = p1.z;
        outputVertices[vertIndex + 3] = 0.;
        outputVertices[vertIndex + 4] = 0.;
        outputVertices[vertIndex + 5] = 0.;
        outputVertices[vertIndex + 6] = p2.x;
        outputVertices[vertIndex + 7] = p2.y;
        outputVertices[vertIndex + 8] = p2.z;
        outputVertices[vertIndex + 9] = 0.;
        outputVertices[vertIndex + 10] = 0.;
        outputVertices[vertIndex + 11] = 0.;
        outputVertices[vertIndex + 12] = p3.x;
        outputVertices[vertIndex + 13] = p3.y;
        outputVertices[vertIndex + 14] = p3.z;
        outputVertices[vertIndex + 15] = 0.;
        outputVertices[vertIndex + 16] = 0.;
        outputVertices[vertIndex + 17] = 0.;

        var triIndex = 3u;//atomicAdd(&countBuffers[1], 3u);
        outputIndices[triIndex + 0] = (vertIndex/6u) + 0;
        outputIndices[triIndex + 1] = (vertIndex/6u) + 1;
        outputIndices[triIndex + 2] = (vertIndex/6u) + 2;
    }
}