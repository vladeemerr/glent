#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace glint::graphics {

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

} // namespace glint::graphics
