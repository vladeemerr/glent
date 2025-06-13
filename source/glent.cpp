#include <stdio.h>
#include <alloca.h>
#include <string.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gles2.h>

namespace {

constexpr int32_t window_default_width = 1280;
constexpr int32_t window_default_height = 720;
constexpr char window_title[] = "Glent";

GLFWwindow* window;

GLuint vertex_shader;
GLuint fragment_shader;
GLuint program;

GLuint compileShader(const char path[], GLenum type) {
	GLuint shader = glCreateShader(type);

	FILE* file = fopen(path, "rb");
	if (!file) {
		fputs("Failed to open vertex shader file\n", stderr);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	int32_t size = ftell(file);
	rewind(file);

	char* text = static_cast<char*>(alloca(size + 1));
	fread((void*)text, size, 1, file);
	text[size] = '\0';

	fclose(file);

	const char* tag = nullptr;
	switch (type) {
	case GL_VERTEX_SHADER:
		tag = "@shader:vertex";
		break;
	case GL_FRAGMENT_SHADER:
		tag = "@shader:fragment";
		break;
	}

	char* code = strstr(text, tag);
	if (code == nullptr) {
		fprintf(stderr, "%s was not found in a file\n", tag);
		return 0;
	}

	while (*code != '\n') {
		++code;
	}

	++code;

	char* end = code;
	while (*end != '@' && *end != '\0') {
		++end;
	}

	int32_t diff = end - code;

	glShaderSource(shader, 1, &code, &diff);
	glCompileShader(shader);

	GLint log_length = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		char* log = static_cast<char*>(alloca(log_length));
		glGetShaderInfoLog(shader, log_length, nullptr, log);
		fprintf(stderr, "%s\n", log);
		return 0;
	}

	return shader;
}

} // namespace

int main() {
	if (!glfwInit()) {
		fputs("Failed to initialize GLFW\n", stderr);
		return 1;
	}

	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_RESIZABLE, false);

	window = glfwCreateWindow(window_default_width, window_default_height,
	                          window_title, nullptr, nullptr);
	if (!window) {
		fputs("Failed to create GLFW window\n", stderr);
		return 1;
	}

	glfwMakeContextCurrent(window);
	gladLoadGLES2(glfwGetProcAddress);
	glfwSwapInterval(1);

	vertex_shader = compileShader("assets/base.glsl", GL_VERTEX_SHADER);
	fragment_shader = compileShader("assets/base.glsl", GL_FRAGMENT_SHADER);

	program = glCreateProgram(); {
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);
		GLint log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0) {
			char* log = static_cast<char*>(alloca(log_length));
			glGetProgramInfoLog(program, log_length, nullptr, log);
			fprintf(stderr, "%s\n", log);
		}

		glDetachShader(program, fragment_shader);
		glDetachShader(program, vertex_shader);
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glfwSwapBuffers(window);
	}

	glDeleteProgram(program);
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
