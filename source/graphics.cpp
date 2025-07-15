#include "graphics.hpp"
#include "graphics_gl.hpp"

#include <vector>

#define GLSL_STD140_ALIGN alignas(16)

namespace glint::graphics {

namespace {

constexpr size_t max_light_count = 16;

constexpr char untextured_unlit_vertex_shader_code[] = R"(
#version 310 es

layout(location = 0) in vec3 v_position;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec4 color;
};

void main() {
	gl_Position = view_projection * transform * vec4(v_position, 1.0f);
}
)";

constexpr char untextured_unlit_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

out vec4 frag_color;

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec4 color;
};

void main() {
	frag_color = color;
}
)";

constexpr char untextured_lit_vertex_shader_code[] = R"(
#version 310 es

struct Light {
	mediump vec3 position;
	mediump vec3 color;
};

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

out vec3 f_position;
out vec3 f_normal;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
	vec3 ambience;
	int light_count;
	Light lights[16];
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec3 diffuse_color;
	vec3 specular_color;
	float roughness;
};

void main() {
	vec4 position = transform * vec4(v_position, 1.0f);
	vec4 normal = transform * vec4(v_normal, 0.0f);

	gl_Position = view_projection * position;
	f_position = position.xyz / position.w;
	f_normal = normalize(normal.xyz);
}
)";

constexpr char untextured_lit_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

struct Light {
	vec3 position;
	vec3 color;
};

in vec3 f_position;
in vec3 f_normal;

out vec4 frag_color;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
	vec3 ambience;
	int light_count;
	Light lights[16];
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec3 diffuse_color;
	vec3 specular_color;
	float roughness;
};

void main() {
	vec3 normal = normalize(f_normal);

	vec3 view_direction = normalize(view_position - f_position);

	vec3 color = ambience * diffuse_color;
	for (int i = 0; i < light_count; ++i) {
		Light light = lights[i];

		vec3 light_direction = normalize(light.position - f_position);
		vec3 half_vector = normalize(view_direction + light_direction);
		float light_distance = distance(light.position, f_position);

		vec3 specular = pow(max(dot(normal, half_vector), 0.0f), roughness) * specular_color;
		
		float coeff = max(dot(normal, light_direction), 0.0f) /
		              (light_distance * light_distance);

		color += (diffuse_color + specular) * light.color * coeff;
	}
	
	frag_color = vec4(color, 1.0f);
}
)";

struct CameraUniforms {
	glm::mat4 view_projection;
	GLSL_STD140_ALIGN glm::vec3 view_position;
	GLSL_STD140_ALIGN glm::vec3 ambience;
	int32_t light_count;
	GLSL_STD140_ALIGN Light lights[max_light_count];
};

struct ModelUniforms {
	glm::mat4 transform;
	GLSL_STD140_ALIGN glm::vec3 diffuse_color;
	GLSL_STD140_ALIGN glm::vec3 specular_color;
	float roughness;
};

gl::Pipeline* untextured_unlit_pipeline;
gl::Pipeline* untextured_lit_pipeline;
gl::Buffer* camera_uniform_buffer;
gl::Buffer* model_uniform_buffer;

CameraUniforms camera_uniforms;
gl::Pipeline* current_pipeline;

} // namespace

void setup() {
	const gl::VertexAttribute attributes[] = {
		{0, GL_FLOAT, 3, false},
		{1, GL_FLOAT, 3, false},
		{2, GL_FLOAT, 2, false},
	};

	const gl::PrimitiveState solid_primitive_state{.mode = GL_TRIANGLES};
	const gl::DepthStencilState solid_depth_stencil_state{.depth_write = true};
	const gl::BlendState solid_blend_state{.enable = false};

	gl::Shader untextured_unlit_vertex_shader(GL_VERTEX_SHADER,
	                                          untextured_unlit_vertex_shader_code);

	gl::Shader untextured_unlit_fragment_shader(GL_FRAGMENT_SHADER,
	                                            untextured_unlit_fragment_shader_code);
	
	untextured_unlit_pipeline = new gl::Pipeline(
		solid_primitive_state, attributes,
		untextured_unlit_vertex_shader, untextured_unlit_fragment_shader,
		solid_depth_stencil_state, solid_blend_state);

	gl::Shader untextured_lit_vertex_shader(GL_VERTEX_SHADER,
	                                        untextured_lit_vertex_shader_code);

	gl::Shader untextured_lit_fragment_shader(GL_FRAGMENT_SHADER,
	                                          untextured_lit_fragment_shader_code);

	untextured_lit_pipeline = new gl::Pipeline(
		solid_primitive_state, attributes,
		untextured_lit_vertex_shader, untextured_lit_fragment_shader,
		solid_depth_stencil_state, solid_blend_state);

	camera_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                       sizeof(CameraUniforms));

	model_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                      sizeof(ModelUniforms));

	current_pipeline = untextured_lit_pipeline;

	camera_uniforms.ambience = {0.1f, 0.1f, 0.1f};
}

void shutdown() {
	delete model_uniform_buffer;
	delete camera_uniform_buffer;
	delete untextured_lit_pipeline;
	delete untextured_unlit_pipeline;
}

void render(const std::span<const Model> models,
            const Camera& camera,
            const std::span<const Light> lights) {
	graphics::gl::clear(0.05f, 0.05f, 0.05f, 1.0f);

	camera_uniforms.view_projection = camera.calculatePerspective();
	camera_uniforms.view_position = camera.position;
	camera_uniforms.light_count = lights.size();
	std::copy_n(lights.begin(), std::min(lights.size(), max_light_count),
	            camera_uniforms.lights);
	camera_uniform_buffer->assign(sizeof(CameraUniforms), &camera_uniforms);
	
	gl::setPipeline(*current_pipeline);
	gl::setUniformBuffer(*camera_uniform_buffer, 0);
	gl::setUniformBuffer(*model_uniform_buffer, 1);

	for (const auto& m : models) {
		ModelUniforms model_uniforms{
			.transform = m.transform,
			.diffuse_color = m.diffuse_color,
			.specular_color = m.specular_color,
			.roughness = m.roughness,
		};
		model_uniform_buffer->assign(sizeof(ModelUniforms), &model_uniforms);
		
		gl::setVertexBuffer(m.mesh.vertexBuffer());
		gl::setIndexBuffer(m.mesh.indexBuffer(), GL_UNSIGNED_INT);

		gl::draw(m.mesh.count());
	}
}

void setRenderMode(const RenderMode mode) {
	switch (mode) {
		case RenderMode::untextured_unlit:
			current_pipeline = untextured_unlit_pipeline;
			break;

		case RenderMode::untextured_lit:
			current_pipeline = untextured_lit_pipeline;
			break;
	}
}

Mesh Mesh::makeCube() {
	const Vertex vertices[] = {
		// Front:
		{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		// Right:
		{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		// Back:
		{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
		// Left:
		{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		// Top:
		{{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		// Bottom:
		{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
	};
	
	const uint32_t indices[] = {
		0, 1, 2,
		3, 2, 1,
		4, 5, 6,
		7, 6, 5,
		8, 9, 10,
		11, 10, 9,
		12, 13, 14,
		15, 14, 13,
		16, 17, 18,
		19, 18, 17,
		20, 21, 22,
		23, 22, 21,
	};
	
	return Mesh(std::span<const Vertex>(vertices),
	            std::span<const uint32_t>(indices));
}

Mesh Mesh::makePlane(glm::vec3 normal) {
	// TODO Use normal to construct plane
	const Vertex vertices[] = {
		{{-0.5f, 0.0f, 0.5f}, normal, {0.0f, 1.0f}},
		{{0.5f, 0.0f, 0.5f}, normal, {1.0f, 1.0f}},
		{{-0.5f, 0.0f, -0.5f}, normal, {0.0f, 0.0f}},
		{{0.5f, 0.0f, -0.5f}, normal, {1.0f, 0.0f}},
	};

	const uint32_t indices[] = {0, 1, 2, 3, 2, 1};

	return Mesh(std::span<const Vertex>(vertices),
	            std::span<const uint32_t>(indices));
}

} // namespace glint::graphics
