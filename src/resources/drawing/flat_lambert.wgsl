struct MyUniforms {
	projectionMatrix: mat4x4f,
	viewMatrix: mat4x4f,
	modelMatrix: mat4x4f, // TODO: inverse model matrix, maybe not necessary for this project so remove if so
	time: f32
};

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f
};

@group(0) @binding(0) var<uniform> u_Uniforms: MyUniforms;

@vertex
fn vs_main(vIn: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	var worldPos = (u_Uniforms.modelMatrix * vec4f(vIn.position, 1.0)).xyz;
	var pos: vec4f = u_Uniforms.projectionMatrix * u_Uniforms.viewMatrix * vec4(worldPos, 1.);

	out.position = pos;
	out.normal = vIn.normal; // inverse model
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	var col = in.normal.xyz*vec3f(0.5)+vec3f(0.5);//vec3f(0.4, 0.4, 0.9);
	var correctedCol = pow(col, vec3f(2.2));
	return vec4(correctedCol, 1.0);
}