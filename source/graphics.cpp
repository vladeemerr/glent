#include "graphics.hpp"

#include "graphics_gl.hpp"

#define GLSL_STD140_ALIGN alignas(16)

namespace glint::graphics {

namespace {

#include "graphics_shaders.hpp"

constexpr size_t max_light_count = 16;
constexpr size_t shadow_map_size = 1024;

struct CameraUniforms {
	glm::mat4 view_projection;
	glm::mat4 shadow_matrix;
	GLSL_STD140_ALIGN glm::vec3 view_position;
	GLSL_STD140_ALIGN glm::vec3 ambience;
	int32_t light_count;
	GLSL_STD140_ALIGN Light lights[max_light_count];
};

struct ModelUniforms {
	glm::mat4 transform;
	GLSL_STD140_ALIGN glm::vec3 albedo_color;
	GLSL_STD140_ALIGN glm::vec3 specular_color;
	float shininess;
	float emissiveness;
};

struct SkyUniforms {
	glm::mat4 view;
	glm::vec2 viewport;
};

struct ShadowMapUniforms {
	glm::mat4 view_projection;
};

gl::Pipeline* pipelines[static_cast<size_t>(RenderMode::count)];
gl::Buffer* camera_uniform_buffer;
gl::Buffer* model_uniform_buffer;

CameraUniforms camera_uniforms;

gl::Buffer* sky_vertex_buffer;
gl::Buffer* sky_uniform_buffer;
gl::Pipeline* sky_pipeline;

gl::Pipeline* shadow_map_pipeline;
gl::Buffer* shadow_map_uniform_buffer;
gl::Texture* shadow_map_texture;
gl::Sampler* shadow_map_sampler;
gl::Framebuffer* shadow_map_framebuffer;

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
	
	pipelines[static_cast<size_t>(RenderMode::untextured_unlit)] = new gl::Pipeline(
		solid_primitive_state, attributes,
		untextured_unlit_vertex_shader, untextured_unlit_fragment_shader,
		solid_depth_stencil_state, solid_blend_state);

	gl::Shader untextured_lit_vertex_shader(GL_VERTEX_SHADER,
	                                        untextured_lit_vertex_shader_code);

	gl::Shader untextured_lit_fragment_shader(GL_FRAGMENT_SHADER,
	                                          untextured_lit_fragment_shader_code);

	pipelines[static_cast<size_t>(RenderMode::untextured_lit)] = new gl::Pipeline(
		solid_primitive_state, attributes,
		untextured_lit_vertex_shader, untextured_lit_fragment_shader,
		solid_depth_stencil_state, solid_blend_state);

	gl::Shader textured_lit_vertex_shader(GL_VERTEX_SHADER,
	                                      textured_lit_vertex_shader_code);

	gl::Shader textured_lit_fragment_shader(GL_FRAGMENT_SHADER,
	                                        textured_lit_fragment_shader_code);

	pipelines[static_cast<size_t>(RenderMode::textured_lit)] = new gl::Pipeline(
		solid_primitive_state, attributes,
		textured_lit_vertex_shader, textured_lit_fragment_shader,
		solid_depth_stencil_state, solid_blend_state);

	camera_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                       sizeof(CameraUniforms));

	model_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                      sizeof(ModelUniforms));

	camera_uniforms.ambience = {0.52f, 0.81f, 0.92f};

	/* Sky */

	const gl::VertexAttribute sky_vertex_attributes[] = {
		{0, GL_FLOAT, 2, false},
	};

	const glm::vec2 sky_vertices[] = {
		{-1.0f, 1.0f},
		{-1.0f, -1.0f},
		{1.0f, 1.0f},
		{1.0f, -1.0f},
	};

	sky_vertex_buffer = new gl::Buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
	                                   sizeof(sky_vertices), sky_vertices);

	sky_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                    sizeof(SkyUniforms));

	gl::Shader sky_vertex_shader(GL_VERTEX_SHADER, sky_vertex_shader_code);
	gl::Shader sky_fragment_shader(GL_FRAGMENT_SHADER, sky_fragment_shader_code);

	sky_pipeline = new gl::Pipeline(
		gl::PrimitiveState{.mode = GL_TRIANGLE_STRIP, .cull_mode = GL_NONE},
		sky_vertex_attributes,
		sky_vertex_shader,
		sky_fragment_shader,
		gl::DepthStencilState{.depth_write = false},
		gl::BlendState{.enable = false});

	/* Shadow map */

	gl::Shader shadow_map_vertex_shader(GL_VERTEX_SHADER,
	                                    shadow_map_vertex_shader_code);

	gl::Shader shadow_map_fragment_shader(GL_FRAGMENT_SHADER,
	                                      shadow_map_fragment_shader_code);

	shadow_map_pipeline = new gl::Pipeline(
		gl::PrimitiveState{.mode = GL_TRIANGLES, .cull_mode = GL_FRONT},
		attributes,
		shadow_map_vertex_shader,
		shadow_map_fragment_shader,
		solid_depth_stencil_state,
		solid_blend_state);

	shadow_map_uniform_buffer = new gl::Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW,
	                                    sizeof(ShadowMapUniforms));

	shadow_map_texture = new gl::Texture(GL_DEPTH_COMPONENT32F,
	                                     shadow_map_size, shadow_map_size);

	shadow_map_sampler = new gl::Sampler({});

	shadow_map_framebuffer = new gl::Framebuffer({}, shadow_map_texture);
}

void shutdown() {
	delete shadow_map_framebuffer;
	delete shadow_map_sampler;
	delete shadow_map_texture;
	delete shadow_map_uniform_buffer;
	delete shadow_map_pipeline;
	
	delete sky_pipeline;
	delete sky_uniform_buffer;
	delete sky_vertex_buffer;
	
	delete model_uniform_buffer;
	delete camera_uniform_buffer;
	delete pipelines[static_cast<size_t>(RenderMode::textured_lit)];
	delete pipelines[static_cast<size_t>(RenderMode::untextured_lit)];
	delete pipelines[static_cast<size_t>(RenderMode::untextured_unlit)];
}

void render(const std::span<const Model> models,
            const Camera& camera,
            const std::span<const Light> lights) {
	gl::setFramebuffer(*shadow_map_framebuffer);
	glViewport(0, 0, shadow_map_size, shadow_map_size);
	gl::clear(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 shadow_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 10.0f);
	glm::mat4 shadow_view = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f),
	                                    glm::vec3(0.0f, 0.0f, 0.0f),
	                                    glm::vec3(0.0f, 1.0f, 0.0f));

	ShadowMapUniforms shadow_map_uniforms{
		.view_projection = shadow_projection * shadow_view,
	};
	shadow_map_uniform_buffer->assign(sizeof(ShadowMapUniforms), &shadow_map_uniforms);

	gl::setPipeline(*shadow_map_pipeline);
	gl::setUniformBuffer(*shadow_map_uniform_buffer, 0);
	gl::setUniformBuffer(*model_uniform_buffer, 1);

	for (const auto& model : models) {
		model_uniform_buffer->assign(sizeof(glm::mat4), &model.transform);

		gl::setVertexBuffer(model.mesh.vertexBuffer());
		gl::setIndexBuffer(model.mesh.indexBuffer(), GL_UNSIGNED_INT);

		gl::draw(model.mesh.count());
	}

	gl::setFramebuffer(gl::Framebuffer::main());
	glViewport(0, 0, camera.viewport.x, camera.viewport.y);
	gl::clear(0.0f, 0.0f, 0.0f, 1.0f);

	SkyUniforms sky_uniforms{
		.view = glm::mat4(mat3_cast(camera.calculateOrientation())),
		.viewport = camera.viewport,
	};
	sky_uniform_buffer->assign(sizeof(SkyUniforms), &sky_uniforms);

	gl::setPipeline(*sky_pipeline);
	gl::setVertexBuffer(*sky_vertex_buffer);
	gl::setUniformBuffer(*sky_uniform_buffer, 0);
	gl::draw(4);

	camera_uniforms.view_projection = camera.calculatePerspective();
	camera_uniforms.shadow_matrix = shadow_map_uniforms.view_projection;
	camera_uniforms.view_position = camera.position;
	camera_uniforms.light_count = lights.size();
	std::copy_n(lights.begin(), std::min(lights.size(), max_light_count),
	            camera_uniforms.lights);
	camera_uniform_buffer->assign(sizeof(CameraUniforms), &camera_uniforms);
	
	for (const auto& model : models) {
		const auto& material = model.material;

		gl::setPipeline(*pipelines[static_cast<size_t>(material.render_mode)]);
		gl::setUniformBuffer(*camera_uniform_buffer, 0);
		gl::setUniformBuffer(*model_uniform_buffer, 1);

		if (material.render_mode == RenderMode::textured_lit) {
			assert(material.texture_sampler != nullptr &&
			       material.albedo_texture != nullptr);
			gl::setTexture(*material.albedo_texture, *material.texture_sampler, 0);
		}

		gl::setTexture(*shadow_map_texture, *shadow_map_sampler, 1);

		ModelUniforms model_uniforms{
			.transform = model.transform,
			.albedo_color = material.albedo_color / glm::pi<float>(),
			.specular_color = material.specular_color *
			                  ((material.shininess + 8.0f) / (8.0f * glm::pi<float>())),
			.shininess = material.shininess,
			.emissiveness = material.emissiveness,
		};
		model_uniform_buffer->assign(sizeof(ModelUniforms), &model_uniforms);
		
		gl::setVertexBuffer(model.mesh.vertexBuffer());
		gl::setIndexBuffer(model.mesh.indexBuffer(), GL_UNSIGNED_INT);

		gl::draw(model.mesh.count());
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
