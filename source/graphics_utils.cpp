#include "graphics_utils.hpp"

#include <glm/ext/vector_float2.hpp>

namespace glint::graphics::utils {

namespace {

constexpr char point_batch_vs_source[] = R"(
#version 310 es

struct Point {
	vec4 position_size;
	vec4 color;
};

layout(location = 0) in vec2 in_position;

flat out vec4 out_color;
out vec2 out_uv;
out flat float out_size;

layout(std140, binding = 0) uniform Uniforms {
	mat4 projected_view;
	vec2 one_over_viewport;
};

layout(std430, binding = 1) readonly buffer Instances {
	Point points[];
};

void main() {
	Point point = points[gl_InstanceID];

	vec4 position = projected_view * vec4(point.position_size.xyz, 1.0f);
	vec2 size = point.position_size.w * one_over_viewport;
	position.xy += in_position * size * position.w;

	gl_Position = position;
	out_color = point.color;
	out_uv = in_position * point.position_size.w;
	out_size = point.position_size.w + 0.5f;
}
)";

constexpr char point_batch_fs_source[] = R"(
#version 310 es
precision mediump float;

flat in vec4 out_color;
in vec2 out_uv;
flat in float out_size;

out vec4 frag_color;

void main() {
	vec4 color = out_color;
	color.a *= clamp(out_size - length(out_uv), 0.0f, 1.0f);

	if (color.a <= 0.0f)
		discard;

	frag_color = color;
}
)";

constexpr char line_batch_vs_source[] = R"(
#version 310 es

struct Point {
	vec4 position_size;
	vec4 color;
};

layout(location = 0) in vec2 in_position;

out vec4 out_color;

layout(std140, binding = 0) uniform Uniforms {
	mat4 projected_view;
	vec2 one_over_viewport;
};

layout(std430, binding = 1) readonly buffer Instances {
	Point points[];
};

void main() {
	Point point = points[2 * gl_InstanceID + (gl_VertexID & 1)];

	vec4 positions[2];
	positions[0] = projected_view * vec4(points[2 * gl_InstanceID].position_size.xyz, 1.0f);
	positions[1] = projected_view * vec4(points[2 * gl_InstanceID + 1].position_size.xyz, 1.0f);

	vec2 dir = normalize((positions[0].xy / positions[0].w) -
	                     (positions[1].xy / positions[1].w));
	vec2 perp = vec2(-dir.y, dir.x);

	vec4 position = positions[gl_VertexID & 1];
	position.xy += perp * in_position.y * position.w *
	               point.position_size.w * one_over_viewport;

	gl_Position = position;
	out_color = point.color;
}
)";

constexpr char line_batch_fs_source[] = R"(
#version 310 es
precision mediump float;

in vec4 out_color;

out vec4 frag_color;

void main() {
	frag_color = out_color;
}
)";

constexpr char polygon_batch_vs_source[] = R"(
#version 310 es

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_color;

out vec4 out_color;

layout(std140, binding = 0) uniform Uniforms {
	mat4 projected_view;
};

void main() {
	gl_Position = projected_view * vec4(in_position.xyz, 1.0f);
	out_color = in_color;
}
)";

constexpr char polygon_batch_fs_source[] = R"(
#version 310 es
precision mediump float;

in vec4 out_color;

out vec4 frag_color;

void main() {
	frag_color = out_color;
}
)";

struct BatchUniforms {
	glm::mat4 projected_view;
	glm::vec2 one_over_viewport;
};

gl::Buffer* unit_quad_vertex_buffer;
gl::Buffer* batch_uniform_buffer;

gl::Shader* point_batch_vertex_shader;
gl::Shader* point_batch_fragment_shader;
gl::Pipeline* point_batch_pipeline;

gl::Shader* line_batch_vertex_shader;
gl::Shader* line_batch_fragment_shader;
gl::Pipeline* line_batch_pipeline;

gl::Shader* polygon_batch_vertex_shader;
gl::Shader* polygon_batch_fragment_shader;
gl::Pipeline* polygon_batch_pipeline;

} // namespace

template<>
void PointBatch::draw(const glm::mat4& projected_view) {
	BatchUniforms uniforms{
		projected_view,
		1.0f / gl::viewport(),
	};
	batch_uniform_buffer->assign(sizeof(uniforms), &uniforms);
	
	gl::setPipeline(*point_batch_pipeline);
	gl::setVertexBuffer(*unit_quad_vertex_buffer);
	gl::setUniformBuffer(*batch_uniform_buffer, 0);
	gl::setStorageBuffer(points_, 1);

	gl::drawInstanced(size_, 4);

	size_ = 0;
}

template<>
void LineBatch::draw(const glm::mat4& projected_view) {
	assert(size_ % 2 == 0);

	BatchUniforms uniforms{
		projected_view,
		1.0f / gl::viewport(),
	};
	batch_uniform_buffer->assign(sizeof(uniforms), &uniforms);
	
	gl::setPipeline(*line_batch_pipeline);
	gl::setVertexBuffer(*unit_quad_vertex_buffer);
	gl::setUniformBuffer(*batch_uniform_buffer, 0);
	gl::setStorageBuffer(points_, 1);

	gl::drawInstanced(size_ / 2, 4);

	size_ = 0;
}

template<>
void PolygonBatch::draw(const glm::mat4& projected_view) {
	assert(size_ % 3 == 0);

	batch_uniform_buffer->assign(sizeof(glm::mat4), &projected_view);
	
	gl::setPipeline(*polygon_batch_pipeline);
	gl::setVertexBuffer(points_);
	gl::setUniformBuffer(*batch_uniform_buffer, 0);

	gl::draw(size_);

	size_ = 0;
}

void setup() {
	const float unit_quad_vertices[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f,
	};

	using namespace glint::graphics::gl;

	const PrimitiveState batch_primitive_state{
		.mode = GL_TRIANGLE_STRIP,
		.cull_mode = GL_NONE,
	};

	const VertexAttribute batch_vertex_layout[] = {
		{0, GL_FLOAT, 2, false},
	};

	const DepthStencilState batch_depth_stencil_state{
		.depth_write = false,
	};

	const BlendState batch_blend_state{
		.enable = true,
		.color_src_factor = GL_SRC_ALPHA,
		.color_dst_factor = GL_ONE_MINUS_SRC_ALPHA,
	};

	unit_quad_vertex_buffer = new Buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
	                                     sizeof(unit_quad_vertices), unit_quad_vertices);

	batch_uniform_buffer = new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                  sizeof(BatchUniforms));

	point_batch_vertex_shader = new Shader(GL_VERTEX_SHADER, point_batch_vs_source);
	point_batch_fragment_shader = new Shader(GL_FRAGMENT_SHADER, point_batch_fs_source);
	point_batch_pipeline = new Pipeline(
		batch_primitive_state,
		batch_vertex_layout,
		*point_batch_vertex_shader,
		*point_batch_fragment_shader,
		batch_depth_stencil_state,
		batch_blend_state);

	line_batch_vertex_shader = new Shader(GL_VERTEX_SHADER, line_batch_vs_source);
	line_batch_fragment_shader = new Shader(GL_FRAGMENT_SHADER, line_batch_fs_source);
	line_batch_pipeline = new Pipeline(
		batch_primitive_state,
		batch_vertex_layout,
		*line_batch_vertex_shader,
		*line_batch_fragment_shader,
		batch_depth_stencil_state,
		batch_blend_state);

	const VertexAttribute polygon_batch_vertex_layout[] = {
		{0, GL_FLOAT, 4, false},
		{1, GL_FLOAT, 4, false},
	};

	polygon_batch_vertex_shader = new Shader(GL_VERTEX_SHADER, polygon_batch_vs_source);
	polygon_batch_fragment_shader = new Shader(GL_FRAGMENT_SHADER, polygon_batch_fs_source);
	polygon_batch_pipeline = new Pipeline(
		PrimitiveState{.mode = GL_TRIANGLES, .cull_mode = GL_NONE},
		polygon_batch_vertex_layout,
		*polygon_batch_vertex_shader,
		*polygon_batch_fragment_shader,
		batch_depth_stencil_state,
		batch_blend_state);
}

void shutdown() {
	delete polygon_batch_pipeline;
	delete polygon_batch_fragment_shader;
	delete polygon_batch_vertex_shader;

	delete line_batch_pipeline;
	delete line_batch_fragment_shader;
	delete line_batch_vertex_shader;

	delete point_batch_pipeline;
	delete point_batch_fragment_shader;
	delete point_batch_vertex_shader;

	delete batch_uniform_buffer;
	delete unit_quad_vertex_buffer;
}

template class Batch<1, GL_SHADER_STORAGE_BUFFER>;
template class Batch<2, GL_SHADER_STORAGE_BUFFER>;
template class Batch<3, GL_ARRAY_BUFFER>;

} // namespace glint::graphics::utils
