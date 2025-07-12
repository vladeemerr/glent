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

	glfwMakeContextCurrent(window);
	if (!gladLoadGLES2(glfwGetProcAddress)) {
		std::cerr << "Failed to load GLES 3.1 functions\n";
		return 1;
	}

	glfwSwapInterval(1);

	graphics::gl::setup(window_default_width, window_default_height);
	graphics::utils::setup();

	while (!glfwWindowShouldClose(window)) {
		input::cache();
		glfwPollEvents();

		float t = float(glfwGetTime());

		graphics::gl::clear(0.05f, 0.05f, 0.05f, 1.0f);

		glfwSwapBuffers(window);
	}

	graphics::utils::shutdown();
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
