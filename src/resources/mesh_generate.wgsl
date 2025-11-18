@group(0) @binding(0) var<storage, read_write> outputVertices: array<f32>; // Try vec3 or smth MAYBE?
@group(0) @binding(1) var<storage, read_write> outputIndices: array<u32>; 
@group(0) @binding(2) var<storage, read_write> countBuffers: array<atomic<u32>>; // (vertCount, indexCount)

const VERTEX_SIZE : u32 = 6u; // 6 Floats per Vertex
fn AddVertex(bufferIndex: u32, pos: vec3f) {
    outputVertices[bufferIndex + 0] = pos.x;
    outputVertices[bufferIndex + 1] = pos.y;
    outputVertices[bufferIndex + 2] = pos.z;
    outputVertices[bufferIndex + 3] = 0.;
    outputVertices[bufferIndex + 4] = 0.;
    outputVertices[bufferIndex + 5] = 0.;
}

fn AddTriangle(bufferIndex: u32, v1: u32, v2: u32, v3: u32) {
    outputIndices[bufferIndex + 0] = v1;
    outputIndices[bufferIndex + 1] = v2;
    outputIndices[bufferIndex + 2] = v3;
}

fn AddVerticesAndTriangle(v1: vec3f, v2: vec3f, v3: vec3f) {
    let bufferVertIndex = atomicAdd(&countBuffers[0], 3*VERTEX_SIZE);
    AddVertex(bufferVertIndex + 0u*VERTEX_SIZE, v1);
    AddVertex(bufferVertIndex + 1u*VERTEX_SIZE, v2);
    AddVertex(bufferVertIndex + 2u*VERTEX_SIZE, v3);

    let bufferTriIndex = atomicAdd(&countBuffers[1], 3u);
    let firstVert = bufferVertIndex/VERTEX_SIZE;
    AddTriangle(bufferTriIndex, firstVert, firstVert+1, firstVert+2);
}

@compute 
@workgroup_size(4, 4, 4) // TODO: make 3D
fn main(@builtin(global_invocation_id) id: vec3<u32>) {
    var i = id.x;
    if(id.y + id.z != 0u) { return; }
    if(i == 1u) {
        let p1 = vec3f(-1.0, -1.0, 0.0);
        let p2 = vec3f(1.0, -1.0, 0.0);
        let p3 = vec3f(-1.0, 1.0, 0.0);
        AddVerticesAndTriangle(p1, p2, p3);
    } else if(i==0u) {
        let p1 = vec3f(1.0, 1.0, 0.0);
        let p2 = vec3f(1.0, -1.0, 0.0);
        let p3 = vec3f(-1.0, 1.0, 0.0);
        AddVerticesAndTriangle(p1, p2, p3);
    }
}