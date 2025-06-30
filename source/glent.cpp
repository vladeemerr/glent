#include <cstdint>
#include <iostream>

#include <alloca.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gles2.h>

#include "graphics.hpp"
using namespace glent;

namespace {

constexpr int32_t window_default_width = 1280;
constexpr int32_t window_default_height = 720;
constexpr char window_title[] = "Glent";

GLFWwindow* window;

} // namespace

int main() {
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
	                          window_title, nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		return 1;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGLES2(glfwGetProcAddress)) {
		std::cerr << "Failed to load GLES 3.1 functions\n";
		return 1;
	}

	glfwSwapInterval(1);

	graphics::setup(window_default_width, window_default_height);
	using graphics::Buffer;
	using graphics::Shader;
	using graphics::VertexAttribute;
	using graphics::Pipeline;

	const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	};

	auto* vertex_buffer = new Buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
	                                 sizeof(vertices), vertices);

	const uint16_t indices[] = {0, 1, 2, 1, 3, 2};

	auto* index_buffer = new Buffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
	                                sizeof(indices), indices);

	const char vs_src[] = R"(
		#version 300 es

		layout(location = 0) in vec3 in_position;
		layout(location = 1) in vec4 in_color;

		out vec4 out_color;

		void main() {
			gl_Position = vec4(in_position, 1.0f);
			out_color = in_color;
		}
	)";

	auto* vertex_shader = new Shader(GL_VERTEX_SHADER, vs_src);

	const char fs_src[] = R"(
		#version 300 es
		precision mediump float;

		in vec4 out_color;

		out vec4 frag_color;

		void main() {
			frag_color = out_color;
		}
	)";

	auto* fragment_shader = new Shader(GL_FRAGMENT_SHADER, fs_src);

	const VertexAttribute attributes[] = {
		{0, GL_FLOAT, 3, false},
		{1, GL_FLOAT, 4, false},
	};

	auto* pipeline = new Pipeline(GL_TRIANGLES, attributes,
	                              *vertex_shader, *fragment_shader);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		graphics::clear(0.05f, 0.05f, 0.05f, 1.0f);

		graphics::setPipeline(*pipeline);
		graphics::setVertexBuffer(*vertex_buffer);
		graphics::setIndexBuffer(*index_buffer, GL_UNSIGNED_SHORT);

		graphics::draw(6);

		glfwSwapBuffers(window);
	}

	delete pipeline;
	delete fragment_shader;
	delete vertex_shader;
	delete index_buffer;
	delete vertex_buffer;

	graphics::shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
