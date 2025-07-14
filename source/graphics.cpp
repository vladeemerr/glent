#include "graphics.hpp"

#include <vector>

namespace glint::graphics {

namespace {

constexpr char untextured_unlit_vertex_shader_code[] = R"(
#version 310 es

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_uv;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec4 color;
};

void main() {
	vec4 position = transform * vec4(v_position, 1.0f);
	vec4 normal = transform * vec4(v_normal, 0.0f);

	gl_Position = view_projection * position;
	f_position = position.xyz;
	f_normal = normal.xyz;
	f_uv = v_uv;
}
)";

constexpr char untextured_unlit_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

in vec3 f_position;
in vec3 f_normal;

out vec4 frag_color;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec4 color;
};

void main() {
	frag_color = color;
}
)";

struct CameraUniforms {
	glm::mat4 view_projection;
	glm::vec3 view_position;
};

struct ModelUniforms {
	glm::mat4 transform;
	glm::vec4 color;
};

gl::Pipeline* untextured_unlit_pipeline;
gl::Buffer* camera_uniform_buffer;
gl::Buffer* model_uniform_buffer;

} // namespace

void setup() {
	const gl::VertexAttribute attributes[] = {
		{0, GL_FLOAT, 3, false},
		{1, GL_FLOAT, 3, false},
		{2, GL_FLOAT, 2, false},
	};

	gl::Shader untextured_unlit_vertex_shader(GL_VERTEX_SHADER,
	                                          untextured_unlit_vertex_shader_code);

	gl::Shader untextured_unlit_fragment_shader(GL_FRAGMENT_SHADER,
	                                            untextured_unlit_fragment_shader_code);
	
	untextured_unlit_pipeline = new gl::Pipeline(
		gl::PrimitiveState{.mode = GL_TRIANGLES},
		attributes, untextured_unlit_vertex_shader, untextured_unlit_fragment_shader,
		gl::DepthStencilState{.depth_write = true}, gl::BlendState{.enable = false});

	camera_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                       sizeof(CameraUniforms));

	model_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                      sizeof(ModelUniforms));
}

void shutdown() {
	delete model_uniform_buffer;
	delete camera_uniform_buffer;
	delete untextured_unlit_pipeline;
}

void render(const std::span<const Model> models, const Camera& camera) {
	graphics::gl::clear(0.05f, 0.05f, 0.05f, 1.0f);

	CameraUniforms camera_uniforms{
		.view_projection = camera.calculatePerspective(),
		.view_position = camera.position,
	};
	camera_uniform_buffer->assign(sizeof(CameraUniforms), &camera_uniforms);
	
	gl::setPipeline(*untextured_unlit_pipeline);
	gl::setUniformBuffer(*camera_uniform_buffer, 0);
	gl::setUniformBuffer(*model_uniform_buffer, 1);

	for (const auto& m : models) {
		ModelUniforms model_uniforms{
			.transform = m.transform,
			.color = m.color,
		};
		model_uniform_buffer->assign(sizeof(ModelUniforms), &model_uniforms);
		
		gl::setVertexBuffer(m.mesh.vertex_buffer);
		gl::setIndexBuffer(m.mesh.index_buffer, GL_UNSIGNED_INT);

		gl::draw(m.mesh.count);
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

} // namespace glint::graphics
