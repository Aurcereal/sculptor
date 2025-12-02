struct MyUniforms {
	projectionMatrix: mat4x4f,
	viewMatrix: mat4x4f,
	modelMatrix: mat4x4f, // TODO: inverse model matrix, maybe not necessary for this project so remove if so
	time: f32
}; // Struct is bigger on CPU, TODO: MAKE IT SAME NAME AS CPU

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f,
	@location(1) worldPosition: vec3f
};

@group(0) @binding(0) var<uniform> u_Uniforms: MyUniforms;
@group(0) @binding(1) var fieldTexture: texture_3d<f32>;
@group(0) @binding(2) var fieldSampler: sampler;

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
@group(0) @binding(3) var<uniform> u_Parameters : Parameters;

@vertex
fn vs_main(vIn: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	let worldPos = (u_Uniforms.modelMatrix * vec4f(vIn.position, 1.0)).xyz;
	var pos: vec4f = u_Uniforms.projectionMatrix * u_Uniforms.viewMatrix * vec4(worldPos, 1.);

	out.position = pos;
	out.normal = vIn.normal; // inverse model
	out.worldPosition = worldPos;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let norm = normalize(in.normal.xyz);

	let lightDir = normalize(u_Parameters.lightDirection);//vec3f(0.0,-1.0,0.0);//-vec3f(1.0)/sqrt(3.0);
	let diffuseColor = textureSample(fieldTexture, fieldSampler, (u_Parameters.bbxInvTRS * vec4(in.worldPosition,1.0)).xyz).rgb;//vec3f(0.3,0.3,0.4);
	let specColor = vec3f(1.0);

	var diffuse = dot(-lightDir, norm);
	diffuse = max(diffuse, 0.0)+0.45;
	let diffuseContrib = diffuse * diffuseColor;

	let camPos = (transpose(u_Uniforms.viewMatrix) * vec4f(-u_Uniforms.viewMatrix[3].xyz, 0.0)).xyz;
	let spec = pow(max(dot(normalize(reflect(in.worldPosition-camPos, norm)), -lightDir), 0.), 32.);
	let specContrib = spec * specColor;

	var col = specContrib + diffuseContrib;// + vec3f(0.2,0.2,0.4);
	var correctedCol = pow(col, vec3f(2.2));
	return vec4(correctedCol, 1.0);
}