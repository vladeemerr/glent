#include "glm/common.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <system_error>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GLAD_GLES2_IMPLEMENTATION
#include <glad/gles2.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {
	constexpr uint16_t default_width = 1280;
	constexpr uint16_t default_height = 720;
	constexpr char default_title[] = "";

	constexpr int32_t water_rows = 10;
	constexpr int32_t water_columns = 10;
}

namespace {
	GLFWwindow *window;

	glm::mat4 camera_projection;
	glm::vec3 camera_origin(0.0f, 2.0f, -8.0f);
	double last_mouse_x, last_mouse_y;
	glm::vec3 camera_rotation;
}

static void onWindowResize(GLFWwindow*, int32_t width, int32_t height) {
	glViewport(0, 0, width, height);
}

static void onMouseMove(GLFWwindow*, double x, double y) {
	camera_rotation.y -= (x - last_mouse_x) * 0.001f;
	camera_rotation.x += (y - last_mouse_y) * 0.001f;
	last_mouse_x = x;
	last_mouse_y = y;
}

static GLuint compileProgram(const std::string_view vertex_shader_path,
                             const std::string_view fragment_shader_path) {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER); {
    	std::filesystem::path path = vertex_shader_path;
		std::ifstream file(path);

		const size_t size = std::filesystem::file_size(path);
		std::string code(size, '\0');

		file.read(code.data(), size);

		const GLchar *const code_cstr = code.c_str();
		glShaderSource(vertex_shader, 1, &code_cstr, nullptr);

		glCompileShader(vertex_shader);

		GLint log_length;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);

		if (log_length > 0) {
			std::string log(log_length, '\0');
			glGetShaderInfoLog(vertex_shader, log_length,
			                   nullptr, log.data());
			throw std::runtime_error(log);
		}
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER); {
    	std::filesystem::path path = fragment_shader_path;
		std::ifstream file(path);

		const size_t size = std::filesystem::file_size(path);
		std::string code(size, '\0');

		file.read(code.data(), size);

		const GLchar *const code_cstr = code.c_str();
		glShaderSource(fragment_shader, 1, &code_cstr, nullptr);

		glCompileShader(fragment_shader);

		GLint log_length;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);

		if (log_length > 0) {
			std::string log(log_length, '\0');
			glGetShaderInfoLog(fragment_shader, log_length,
			                   nullptr, log.data());
			throw std::runtime_error(log);
		}
	}

	GLuint program = glCreateProgram(); {
		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);

		glLinkProgram(program);

		GLint log_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

		if (log_length > 0) {
			std::string log(log_length, '\0');
			glGetProgramInfoLog(program, log_length, nullptr, log.data());
			throw std::runtime_error(log);
		}
	}

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

    return program;
}

int main() try {
	if (!glfwInit()) {
		const char *desc = nullptr;
		int32_t code = glfwGetError(&desc);
		throw std::system_error(code, std::generic_category(), desc);
	}

	glfwWindowHint(GLFW_RESIZABLE, false);

	glfwWindowHint(GLFW_DOUBLEBUFFER, true);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifndef NDEBUG
	glfwWindowHint(GLFW_CONTEXT_DEBUG, true);
#endif

	window = glfwCreateWindow(default_width, default_height,
	                          default_title, nullptr, nullptr);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, onMouseMove);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLES2(glfwGetProcAddress)) {
		throw std::runtime_error("Failed to load GLES3");
	}

	glfwSetWindowSizeCallback(window, onWindowResize);

	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(4.0f);

	////////////////
	/* Rock block */
	////////////////

	GLuint cube_vao;
	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);

	GLuint cube_vbo; {
		glGenBuffers(1, &cube_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);

		const float cube_vertices[] = {
			// Front
			+0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
			+0.5f, +0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
			-0.5f, +0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
			-0.5f, +0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
			+0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,

			// Right
			+0.5f, -0.5f, -0.5f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			+0.5f, -0.5f, +0.5f, +1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			+0.5f, +0.5f, +0.5f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			+0.5f, +0.5f, +0.5f, +1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			+0.5f, +0.5f, -0.5f, +1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			+0.5f, -0.5f, -0.5f, +1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

			// Back
			+0.5f, -0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 1.0f, 1.0f,
			-0.5f, +0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 1.0f, 0.0f,
			-0.5f, +0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 1.0f, 0.0f,
			+0.5f, +0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
			+0.5f, -0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f,

			// Left
			-0.5f, -0.5f, +0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f, +0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			-0.5f, +0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			-0.5f, +0.5f, +0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, +0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

			// Top
			-0.5f, +0.5f, -0.5f, 0.0f, +1.0f, 0.0f, 0.0f, 1.0f,
			+0.5f, +0.5f, -0.5f, 0.0f, +1.0f, 0.0f, 1.0f, 1.0f,
			+0.5f, +0.5f, +0.5f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f,
			+0.5f, +0.5f, +0.5f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f,
			-0.5f, +0.5f, +0.5f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f,
			-0.5f, +0.5f, -0.5f, 0.0f, +1.0f, 0.0f, 0.0f, 1.0f,

			// Bottom
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, +0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
			+0.5f, -0.5f, +0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			+0.5f, -0.5f, +0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			+0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices),
		             cube_vertices, GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * sizeof(float),
	                      reinterpret_cast<void*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 8 * sizeof(float),
	                      reinterpret_cast<void*>(3 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 8 * sizeof(float),
	                      reinterpret_cast<void*>(6 * sizeof(float)));

	GLuint cube_shader = compileProgram("./assets/vs_base.glsl",
	                                    "./assets/fs_base.glsl");
	glUseProgram(cube_shader);

	GLuint cube_texture; {
		glGenTextures(1, &cube_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cube_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int32_t width, height, channels;
		uint8_t *pixels = stbi_load("./assets/rock.png",
		                            &width, &height, &channels, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		             GL_UNSIGNED_BYTE, pixels);

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(pixels);
	}

	GLuint cube_tint = glGetUniformLocation(cube_shader, "u_tint");
	glUniform4f(cube_tint, 1.0f, 1.0f, 1.0f, 1.0f);

	GLuint cube_view_projection = glGetUniformLocation(cube_shader, "u_view_projection");
	GLuint cube_model = glGetUniformLocation(cube_shader, "u_model");

	GLuint cube_sampler = glGetUniformLocation(cube_shader, "u_texture");
	glUniform1i(cube_sampler, 0);

	GLuint cube_view_position = glGetUniformLocation(cube_shader, "u_view_position");

	///////////
	/* Water */
	///////////

	GLuint water_vao;
	glGenVertexArrays(1, &water_vao);
	glBindVertexArray(water_vao);

	GLuint water_vbo;
	glGenBuffers(1, &water_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, water_vbo);

	std::vector<glm::vec3> water_vertices;
	std::vector<uint32_t> water_indices;

	for (int32_t y = 0; y < water_rows; ++y) {
		for (int32_t x = 0; x < water_columns; ++x) {
			water_vertices.emplace_back(y, 0.0f, x);
		}
	}

	glBufferData(GL_ARRAY_BUFFER, water_vertices.size() * sizeof(glm::vec3),
	             water_vertices.data(), GL_STATIC_DRAW);

	GLuint water_ibo;
	glGenBuffers(1, &water_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, water_ibo);

	for (int32_t y = 0; y < water_rows - 1; ++y) {
		water_indices.push_back(y * water_columns);
		for (int32_t x = 0; x < water_columns; ++x) {
			water_indices.push_back(y * water_columns + x);
			water_indices.push_back((y + 1) * water_columns + x);
		}
		water_indices.push_back((y + 1) * water_columns + water_columns - 1);
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, water_indices.size() * sizeof(uint32_t),
	             water_indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float),
	                      reinterpret_cast<void*>(0));

	GLuint water_shader = compileProgram("./assets/vs_water.glsl",
	                                     "./assets/fs_water.glsl");
	glUseProgram(water_shader);

	GLuint water_view_projection = glGetUniformLocation(water_shader, "u_view_projection");

	GLuint water_model = glGetUniformLocation(water_shader, "u_model"); {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-(water_rows - 1) / 2.0f, 0.0f, -(water_columns - 1) / 2.0f));
		glUniformMatrix4fv(water_model, 1, false, glm::value_ptr(model));
	}

	GLuint water_color = glGetUniformLocation(water_shader, "u_color");
	GLuint water_time = glGetUniformLocation(water_shader, "u_time");

	// NOTE: Other variables

	camera_projection = glm::perspectiveFovLH(
		glm::radians(60.0f), float(default_width), float(default_height),
		0.01f, 100.0f);

	while (!glfwWindowShouldClose(window)) {
		float time = glfwGetTime();
		glfwPollEvents();

		// NOTE: Camera movement

		const float pi = glm::pi<float>();
		camera_rotation.y = glm::mod(camera_rotation.y, 2 * pi);
		if (camera_rotation.x < -pi * 0.45f)
			camera_rotation.x = -pi * 0.45f;
		else if (camera_rotation.x > pi * 0.45f)
			camera_rotation.x = pi * 0.45f;

		glm::vec3 camera_forward = glm::normalize(glm::vec3(
			-glm::cos(camera_rotation.x) * glm::sin(camera_rotation.y),
			-glm::sin(camera_rotation.x),
			 glm::cos(camera_rotation.x) * glm::cos(camera_rotation.y)
		));
		glm::vec3 camera_right = glm::normalize(glm::cross(
			glm::vec3(0.0f, 1.0f, 0.0f), camera_forward));
		glm::vec3 camera_up = glm::normalize(glm::cross(
			camera_forward, camera_right));

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera_origin += camera_forward * 0.1f;

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera_origin -= camera_forward * 0.1f;

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera_origin += camera_right * 0.1f;

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera_origin -= camera_right * 0.1f;

		// NOTE: View-projection matrix construction

		glm::mat4 view = glm::lookAtLH(camera_origin, camera_origin + camera_forward, camera_up);
		glm::mat4 view_projection = camera_projection * view;

		// NOTE: Clear back buffer

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// NOTE: Draw rock

		glBindVertexArray(cube_vao);
		glUseProgram(cube_shader);
		glEnable(GL_CULL_FACE);
		glUniformMatrix4fv(cube_view_projection, 1, false, glm::value_ptr(view_projection));
		{
			glm::mat4 model = glm::mat4(1.0);
			model = glm::rotate(model, time * 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(0.0f, glm::sin(time * 0.5f) * 0.1f, 0.0f));
			glUniformMatrix4fv(cube_model, 1, false, glm::value_ptr(model));
		}
		glUniform3f(cube_view_position, camera_origin.x, camera_origin.y, camera_origin.z);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		{
			glm::mat4 model = glm::mat4(1.0);
			model = glm::rotate(model, -time * 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::translate(model, glm::vec3(2.0f, glm::cos(time * 0.5f) * 0.1f, 0.0f));
			glUniformMatrix4fv(cube_model, 1, false, glm::value_ptr(model));
		}
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// NOTE: Draw water

	#if 1
		glBindVertexArray(water_vao);
		glUseProgram(water_shader);
		glDisable(GL_CULL_FACE);
		glUniformMatrix4fv(water_view_projection, 1, false, glm::value_ptr(view_projection));
		glUniform1f(water_time, time);
		glUniform4f(water_color, 0.2f, 0.2f, 0.9f, 0.5f);
		glDrawElements(GL_TRIANGLE_STRIP, water_indices.size(),
		               GL_UNSIGNED_INT, nullptr);

		glUniform4f(water_color, 0.4f, 0.4f, 1.0f, 1.0f);
		glDrawElements(GL_LINE_STRIP, water_indices.size(),
		               GL_UNSIGNED_INT, nullptr);
	#endif

		glfwSwapBuffers(window);
	}

	glfwTerminate();
} catch (const std::exception &e) {
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Unhandled exception caught!" << std::endl;
	return EXIT_FAILURE;
}
