#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLAD_GLES2_IMPLEMENTATION
#include <glad/gles2.h>

namespace {
	constexpr uint32_t default_width = 1280;
	constexpr uint32_t default_height = 720;
	constexpr char default_title[] = "Glent";

	GLFWwindow *window;
} // namespace

int main() {
	if (!glfwInit()) {
		const char *desc = nullptr;
		glfwGetError(&desc);
		std::cerr << "error: failed to initialize GLFW: " << desc << std::endl;
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifndef NDEBUG
	glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#endif

	window = glfwCreateWindow(default_width, default_height,
	                          default_title, nullptr, nullptr);
	if (!window) {
		const char *desc = nullptr;
		glfwGetError(&desc);
		std::cerr << "error: failed to create GLFW window: " << desc << std::endl;
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLES2(glfwGetProcAddress)) {
		std::cerr << "error: failed to initialize GLAD loader" << std::endl;
		return EXIT_FAILURE;
	}

	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
}
