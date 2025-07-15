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

enum class RenderMode {
	untextured_unlit,
	untextured_lit,
};

struct Vertex final {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct Mesh final {
public:
	Mesh() = delete;
	Mesh(const std::span<const Vertex> vertices,
	     const std::span<const uint32_t> indices)
	: vertex_buffer_(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
	                 vertices.size() * sizeof(Vertex), vertices.data()),
	  index_buffer_(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
	                indices.size() * sizeof(uint32_t), indices.data()),
	  count_{static_cast<uint32_t>(indices.size())} {}

	const gl::Buffer& vertexBuffer() const & noexcept { return vertex_buffer_; }
	const gl::Buffer& indexBuffer() const & noexcept { return index_buffer_; }
	uint32_t count() const { return count_; }

	static Mesh makeCube();
	static Mesh makePlane(glm::vec3 normal);

private:
	gl::Buffer vertex_buffer_;
	gl::Buffer index_buffer_;
	uint32_t count_;
};

struct Model final {
	const Mesh& mesh;
	glm::mat4 transform;
	glm::vec3 diffuse_color;
	glm::vec3 specular_color;
	float shininess;
};

struct Light final {
	glm::vec3 position;
	float size;
	alignas(16) glm::vec3 color;
};

struct Camera final {
public:
	static constexpr float default_fov = 70.0f;
	static constexpr float default_near_plane = 0.001f;
	static constexpr float default_far_plane = 1000.0f;

	glm::vec2 viewport;
	float fov = default_fov;
	glm::vec3 position = glm::vec3();
	glm::vec3 rotation = glm::vec3();

	glm::quat calculateOrientation() const {
		glm::quat pitch = glm::angleAxis(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yaw = glm::angleAxis(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat roll = glm::angleAxis(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

		return glm::normalize(roll * yaw * pitch);
	}

	glm::mat4 calculateView() const {
		glm::quat orientation = calculateOrientation();
		glm::mat4 rotation = glm::mat4_cast(orientation);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), position) * rotation;
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

void render(const std::span<const Model> models,
            const Camera& camera,
            const std::span<const Light> lights);
void setRenderMode(const RenderMode mode);

} // namespace glint::graphics
