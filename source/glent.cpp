#include <cstdint>
#include <iostream>

#include <alloca.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gles2.h>

namespace {

constexpr int32_t window_default_width = 1280;
constexpr int32_t window_default_height = 720;
constexpr char window_title[] = "Glent";

GLFWwindow* window;

void GLAPIENTRY glDebugCallback(GLenum /*source*/, GLenum type,
                                GLuint /*id*/, GLenum /*severity*/,
                                GLsizei /*length*/, const GLchar* message,
                                const void* /*user_param*/) {
	if (type != GL_DEBUG_TYPE_ERROR_KHR) {
		return;
	}

	std::cerr << "GLES ERROR: " << message << '\n';
}

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

	glEnable(GL_DEBUG_OUTPUT_KHR);
	glDebugMessageCallbackKHR(glDebugCallback, 0);

	const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	const uint16_t indices[] = {0, 1, 2};

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glVertexAttribBinding(0, 0);
	glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribBinding(1, 0);
	glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

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

	const GLchar* vs_src_str = vs_src;
	GLint vs_src_length = sizeof(vs_src);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_src_str, &vs_src_length);
	glCompileShader(vs);

	GLint vs_status;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_status);
	if (!vs_status) {
		GLint log_length;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &log_length);

		char* log = static_cast<char*>(alloca(log_length));
		glGetShaderInfoLog(vs, log_length, nullptr, log);

		std::cerr << "Vertex shader compilation error:\n" << log << '\n';
	}

	const char fs_src[] = R"(
		#version 300 es
		precision mediump float;

		in vec4 out_color;

		out vec4 frag_color;

		void main() {
			frag_color = out_color;
		}
	)";

	const GLchar* fs_src_str = fs_src;
	GLint fs_src_length = sizeof(fs_src);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_src_str, &fs_src_length);
	glCompileShader(fs);

	GLint fs_status;
	glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_status);
	if (!fs_status) {
		GLint log_length;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &log_length);

		char* log = static_cast<char*>(alloca(log_length));
		glGetShaderInfoLog(fs, log_length, nullptr, log);

		std::cerr << "Fragment shader compilation error:\n" << log << '\n';
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint program_status;
	glGetProgramiv(program, GL_LINK_STATUS, &program_status);
	if (!program_status) {
		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

		char* log = static_cast<char*>(alloca(log_length));
		glGetProgramInfoLog(program, log_length, nullptr, log);

		std::cerr << "Shader linking error:\n" << log << '\n';
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glBindVertexBuffer(0, vbo, 0, 7 * sizeof(float));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);

		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
