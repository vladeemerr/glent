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

	auto* cube_mesh = new graphics::Mesh(graphics::Mesh::makeCube());
	auto* plane_mesh = new graphics::Mesh(graphics::Mesh::makePlane({0.0f, 1.0f, 0.0f}));

	std::vector<graphics::Model> models{
		{
			.mesh = *cube_mesh,
			.transform = glm::mat4(1.0f),
			.diffuse_color = glm::vec3(1.0f, 1.0f, 1.0f),
			.specular_color = glm::vec3(0.0f, 0.0f, 0.0f),
			.shininess = 1.0f,
		},
		{
			.mesh = *plane_mesh,
			.transform = glm::scale(glm::mat4(1.0f), {10.0f, 10.0f, 10.0f}),
			.diffuse_color = glm::vec3(1.0f, 1.0f, 1.0f),
			.specular_color = glm::vec3(1.0f, 1.0f, 1.0f),
			.shininess = 32.0f,
		},
	};

	std::vector<graphics::Light> lights{
		{{}, 1.5f, {1.0f, 1.0f, 1.0f}}
	};

	while (!glfwWindowShouldClose(window)) {
		input::cache();
		glfwPollEvents();

		camera.rotation.x -= input::mouse::cursorDelta().y * 0.01f;
		camera.rotation.x = glm::clamp(camera.rotation.x,
		                               -glm::pi<float>() / 2.0f,
		                               glm::pi<float>() / 2.0f);

		camera.rotation.y -= input::mouse::cursorDelta().x * 0.01f;
		camera.rotation.y = glm::mod(camera.rotation.y, 2.0f * glm::pi<float>());

		glm::quat orientation = camera.calculateOrientation();
		glm::quat forward_orient = orientation *
		                           glm::quat(0.0f, {0.0f, 0.0f, -1.0f}) *
		                           glm::conjugate(orientation);
		glm::vec3 forward = glm::normalize(glm::vec3(forward_orient.x,
		                                             forward_orient.y,
		                                             forward_orient.z));
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		glm::vec3 right = glm::normalize(glm::cross(forward, up));

		if (input::keyboard::isKeyDown(input::keyboard::Key::w)) {
			camera.position += forward * 0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::s)) {
			camera.position -= forward * 0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::a)) {
			camera.position -= right * 0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::d)) {
			camera.position += right * 0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::q)) {
			camera.position += up * 0.1f;
		}

		if (input::keyboard::isKeyDown(input::keyboard::Key::z)) {
			camera.position -= up * 0.1f;
		}

		float t = float(glfwGetTime());

		lights[0].position = {2.0f * glm::cos(t), 1.0f, 2.0f * glm::sin(t)};

		models[0].transform = glm::rotate(glm::translate(glm::mat4(1.0f), {0.0f, 1.0f, 0.0f}),
		                                  t, {1.0f, 1.0f, 1.0f});
		models[0].diffuse_color.r = 0.5f + glm::sin(t) * 0.5f;
		models[0].diffuse_color.g = 0.5f + glm::cos(t) * 0.5f;
		models[0].diffuse_color.b = 0.5f + glm::sin(t) * glm::cos(t) * 0.5f;
		graphics::render(models, camera, lights);

		glfwSwapBuffers(window);
	}

	delete plane_mesh;
	delete cube_mesh;

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
