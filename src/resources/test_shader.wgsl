struct MyUniforms {
	projectionMatrix: mat4x4f,
	viewMatrix: mat4x4f,
	modelMatrix: mat4x4f, // TODO: inverse model matrix, maybe not necessary for this project so remove if so
	color: vec4f,
	time: f32
};

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f
};

struct VertexOutput {
	@builtin(position) position: vec4f,
	@location(0) normal: vec3f,//@location(0) color: vec3f, bring norm back
	@location(1) uv: vec2f
};

@group(0) @binding(0) var<uniform> u_Uniforms: MyUniforms;
@group(0) @binding(1) var testTexture: texture_3d<f32>;
@group(0) @binding(2) var testSampler: sampler;

fn rot(v: vec2f, o: f32) -> vec2f {
	return mat2x2f(cos(o), sin(o), -sin(o), cos(o)) * v;
}

@vertex
fn vs_main(vIn: VertexInput) -> VertexOutput {
	var out: VertexOutput;
	var worldPos = (u_Uniforms.modelMatrix * vec4f(vIn.position, 1.0)).xyz;
	var pos: vec4f = u_Uniforms.projectionMatrix * u_Uniforms.viewMatrix * vec4(worldPos, 1.);

	out.position = pos;
	out.normal = vIn.normal; // inverse model
	out.uv = vIn.position.xy*0.5+0.5;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	var col = in.normal.xyz;//*vec3f(0.5)+vec3f(0.5);//vec3f(0.4, 0.4, 0.9);
	var correctedCol = pow(col, vec3f(2.2));
	return vec4(correctedCol, 1.0);
}