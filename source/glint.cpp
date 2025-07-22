#include <cstdint>
#include <iostream>
#include <array>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gles2.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <lodepng.h>

#include "input.hpp"
#include "graphics.hpp"
#include "graphics_gl.hpp"
#include "graphics_utils.hpp"
using namespace glint;

namespace {

constexpr int32_t window_default_width = 1280;
constexpr int32_t window_default_height = 720;

GLFWwindow* window;

} // namespace

int main() try {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return 1;
	}

	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_DOUBLEBUFFER, true);

	window = glfwCreateWindow(window_default_width, window_default_height,
	                          "Glint", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		return 1;
	}

	input::setup(window);
	input::mouse::setCaptured(true);

	glfwMakeContextCurrent(window);
	if (!gladLoadGLES2(glfwGetProcAddress)) {
		std::cerr << "Failed to load GLES 3.1 functions\n";
		return 1;
	}

	glfwSwapInterval(1);

	graphics::gl::setup(window_default_width, window_default_height);
	graphics::setup();
	graphics::utils::setup();

	graphics::Camera camera{
		.viewport = glm::vec2(window_default_width, window_default_height),
		.fov = 70.0f,
		.position = glm::vec3{0.0f, 1.0f, 2.0f},
	};

	auto* texture_sampler = new graphics::gl::Sampler({
		.min_filter = GL_LINEAR_MIPMAP_LINEAR,
		.anisotropy = 16.0f,
	});

	auto* floor_texture = []() -> graphics::gl::Texture* {
		uint32_t texture_width, texture_height;
		std::vector<uint8_t> texture_data;
		lodepng::decode(texture_data, texture_width, texture_height, "./assets/floor.png");
		return new graphics::gl::Texture(GL_RGBA8,
		                                 texture_width, texture_height,
		                                 texture_data.data());
	}();

	auto* cube_texture = []() -> graphics::gl::Texture* {
		uint32_t texture_width, texture_height;
		std::vector<uint8_t> texture_data;
		lodepng::decode(texture_data, texture_width, texture_height, "./assets/maxwell-nowhiskers.png");
		return new graphics::gl::Texture(GL_RGBA8,
		                                 texture_width, texture_height,
		                                 texture_data.data());
	}();

	auto* cube_mesh = new graphics::Mesh(graphics::Mesh::makeCube());
	auto* plane_mesh = new graphics::Mesh(graphics::Mesh::makePlane({0.0f, 1.0f, 0.0f}));

	graphics::Material cube_material{
		.render_mode = graphics::RenderMode::textured_lit,
		.albedo_color = glm::vec3(1.0f),
		.texture_sampler = texture_sampler,
		.albedo_texture = cube_texture,
	};

	graphics::Material floor_material{
		.render_mode = graphics::RenderMode::textured_lit,
		.specular_color = glm::vec3(1.0f),
		.shininess = 16.0f,
		.texture_sampler = texture_sampler,
		.albedo_texture = floor_texture,
	};

	std::vector<graphics::Model> models{
		{
			.mesh = *cube_mesh,
			.material = cube_material,
			.transform = glm::mat4(1.0f),
		},
		{
			.mesh = *plane_mesh,
			.material = floor_material,
			.transform = glm::scale(glm::mat4(1.0f), {10.0f, 10.0f, 10.0f}),
		},
	};

	std::vector<graphics::Light> lights{
		{{}, 2.0f, {1.0f, 0.3f, 0.3f}},
		{{}, 2.0f, {0.3f, 1.0f, 0.3f}},
		{{}, 2.0f, {0.3f, 0.3f, 1.0f}},
	};

	while (!glfwWindowShouldClose(window)) {
		input::cache();
		glfwPollEvents();

		static glm::vec3 camera_forward_speed{};
		static glm::vec3 camera_right_speed{};
		static glm::vec3 camera_up_speed{};

		if (input::keyboard::isKeyDown(input::keyboard::Key::up)) {
			camera.rotation.x += 0.05f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::down)) {
			camera.rotation.x -= 0.05f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::right)) {
			camera.rotation.y -= 0.05f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::left)) {
			camera.rotation.y += 0.05f;
		}

		camera.rotation.x -= input::mouse::cursorDelta().y * 0.005f;
		camera.rotation.x = glm::clamp(camera.rotation.x,
		                               -glm::pi<float>() / 2.0f,
		                               glm::pi<float>() / 2.0f);

		camera.rotation.y -= input::mouse::cursorDelta().x * 0.005f;
		camera.rotation.y = glm::mod(camera.rotation.y, 2.0f * glm::pi<float>());

		glm::mat3 rotation = mat3_cast(camera.calculateOrientation());
		glm::vec3 forward = glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));

		if (input::keyboard::isKeyDown(input::keyboard::Key::w)) {
			camera_forward_speed = forward * 0.1f;
		} else if (input::keyboard::isKeyDown(input::keyboard::Key::s)) {
			camera_forward_speed = forward * -0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::a)) {
			camera_right_speed = right * -0.1f;
		} else if (input::keyboard::isKeyDown(input::keyboard::Key::d)) {
			camera_right_speed = right * 0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::q)) {
			camera_up_speed = up * 0.1f;
		} else if (input::keyboard::isKeyDown(input::keyboard::Key::z)) {
			camera_up_speed = up * -0.1f;
		}

		camera.position += camera_forward_speed + camera_right_speed + camera_up_speed;
		camera_forward_speed *= 0.8f;
		camera_right_speed *= 0.8f;
		camera_up_speed *= 0.8f;

		float t = float(glfwGetTime());

		float theta = 2.0f * glm::pi<float>() / lights.size();
		for (size_t i = 0; i < lights.size(); ++i) {
			float angle = t + theta * i;
			lights[i].position = {2.0f * glm::cos(angle), 1.5f, 2.0f * glm::sin(angle)};
		}

		models[0].transform = glm::rotate(glm::translate(glm::mat4(1.0f), {0.0f, 1.0f, 0.0f}),
		                                  t, glm::normalize(glm::vec3{glm::cos(t), glm::sin(t),
		                                                              glm::cos(t) * glm::sin(t)}));

		graphics::render(models, camera, lights);

		glfwSwapBuffers(window);
	}

	delete plane_mesh;
	delete cube_mesh;

	delete floor_texture;
	delete texture_sampler;

	graphics::utils::shutdown();
	graphics::shutdown();
	graphics::gl::shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
} catch (const std::runtime_error& e) {
	std::cerr << e.what() << '\n';
	return 1;
} catch (...) {
	std::cerr << "Foreign exception caught!\n";
	return 1;
}
