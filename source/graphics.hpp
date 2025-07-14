#pragma once

#include <cstdint>
#include <span>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "graphics_gl.hpp"

namespace glint::graphics {

struct Vertex final {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct Mesh final {
	gl::Buffer vertex_buffer;
	gl::Buffer index_buffer;
	uint32_t count;

	Mesh(const std::span<const Vertex> vertices,
	     const std::span<const uint32_t> indices)
	: vertex_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
	                vertices.size() * sizeof(Vertex), vertices.data()),
	  index_buffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
	               indices.size() * sizeof(uint32_t), indices.data()),
	  count{static_cast<uint32_t>(indices.size())} {}

	static Mesh makeCube();
};

struct Model final {
	const Mesh& mesh;
	glm::mat4 transform;
	glm::vec4 color;
};

struct Camera final {
public:
	static constexpr float default_fov = 70.0f;
	static constexpr float default_near_plane = 0.001f;
	static constexpr float default_far_plane = 1000.0f;

	glm::vec2 viewport;
	float fov = default_fov;
	glm::vec3 position = glm::vec3();
	glm::quat orientation = glm::quat(1.0f, {});

	glm::mat4 calculateView() const {
		glm::mat4 model = glm::translate(glm::mat4_cast(orientation), position);
		return glm::inverse(model);
	}

	glm::mat4 calculatePerspective() const {
		glm::mat4 perspective  = glm::perspective(fov, viewport.x / viewport.y,
		                                          default_near_plane, default_far_plane);
		glm::mat4 view = calculateView();
		return perspective * view;
	}

	glm::mat4 calculateOrthographic() const {
		glm::mat4 orthographic = glm::ortho(0.0f, viewport.x, viewport.y, 0.0f);
		glm::mat4 view = calculateView();
		return orthographic * view;
	}
};

void setup();
void shutdown();

void render(const std::span<const Model> models, const Camera& camera);

} // namespace glint::graphics
