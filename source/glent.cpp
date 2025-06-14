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
GLuint draw_program;

GLuint compute_shader;
GLuint compute_program;
GLuint texture;

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
	case GL_COMPUTE_SHADER:
		tag = "@shader:compute";
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

GLuint linkProgram(const GLuint shaders[], uint32_t n_shaders) {
	GLuint program = glCreateProgram();

	for (uint32_t i = 0; i < n_shaders; ++i) {
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);

	GLint log_length = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0) {
		char* log = static_cast<char*>(alloca(log_length));
		glGetProgramInfoLog(program, log_length, nullptr, log);
		fprintf(stderr, "%s\n", log);
		return 0;
	}

	for (uint32_t i = 0; i < n_shaders; ++i) {
		glDetachShader(program, shaders[i]);
	}

	return program;
}

void GLAPIENTRY glDebugCallback(GLenum /*source*/, GLenum type,
                                GLuint /*id*/, GLenum severity,
                                GLsizei /*length*/, const GLchar* message,
                                const void* /*user_param*/) {
	if (type != GL_DEBUG_TYPE_ERROR_KHR)
		return;

	fprintf(stderr, "GL ERROR: type = 0x%x, severity = 0x%x, message = %s\n",
	        type, severity, message);
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

	glEnable(GL_DEBUG_OUTPUT_KHR);
	glDebugMessageCallbackKHR(glDebugCallback, 0);

	vertex_shader = compileShader("assets/base.glsl", GL_VERTEX_SHADER);
	fragment_shader = compileShader("assets/base.glsl", GL_FRAGMENT_SHADER);
	GLuint draw_shaders[] = {vertex_shader, fragment_shader};
	draw_program = linkProgram(draw_shaders, 2);

	compute_shader = compileShader("assets/rt.glsl", GL_COMPUTE_SHADER);
	compute_program = linkProgram(&compute_shader, 1);
	glUseProgram(compute_program);

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F,
	               window_default_width, window_default_height);
	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glUseProgram(compute_program);
		glDispatchCompute(window_default_width, window_default_height, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glUseProgram(draw_program);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glfwSwapBuffers(window);
	}

	glDeleteTextures(1, &texture);

	glDeleteShader(compute_shader);
	glDeleteProgram(compute_program);

	glDeleteProgram(draw_program);
	glDeleteShader(fragment_shader);
	glDeleteShader(vertex_shader);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
