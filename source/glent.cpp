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
	using graphics::Point;
	using graphics::PointBatch;
	using graphics::LineBatch;
	using graphics::PolygonBatch;

	auto* point_batch = new PointBatch(16);
	std::vector<Point> points{
		{{0.0f, 1.0f, 0.0f}, 16.0f, {1.0f, 1.0f, 1.0f, 1.0f}},
	};

	auto* line_batch = new LineBatch(16);
	std::vector<Point> line_points;

	auto* polygon_batch = new PolygonBatch(32);
	std::vector<Point> polygon_points;

	float dr = 2.0f * glm::pi<float>() / 3.0f;
	for (int32_t i = 0; i < 3; ++i) {
		float r = dr * i;

		Point p0{glm::vec3{glm::cos(r), -1.0f, glm::sin(r)}, 0.0f,
		         glm::vec4{1.0f, 0.0f, 0.0f, 0.5f}};

		Point p1{glm::vec3{glm::cos(r + dr), -1.0f, glm::sin(r + dr)}, 0.0f,
		         glm::vec4{0.0f, 1.0f, 0.0f, 0.5f}};

		Point p2{glm::vec3{0.0f, 1.0f, 0.0f}, 0.0f,
		         glm::vec4{0.0f, 0.0f, 1.0f, 0.5f}};

		points.emplace_back(p0.position, 16.0f, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});

		polygon_points.push_back(p0);
		polygon_points.push_back(p1);
		polygon_points.push_back(p2);

		line_points.emplace_back(p0.position, 3.0f, p0.color + 0.5f);
		line_points.emplace_back(p1.position, 3.0f, p1.color + 0.5f);
		line_points.emplace_back(p1.position, 3.0f, p1.color + 0.5f);
		line_points.emplace_back(p2.position, 3.0f, p2.color + 0.5f);
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float t = float(glfwGetTime());

		auto projection = glm::perspectiveLH(
			glm::radians(75.0f),
			float(window_default_width) / window_default_height,
			0.001f, 1000.0f);
		auto view = glm::translate(glm::rotate(glm::mat4(1.0f), t, {0.0f, 1.0f, 0.0f}), {0.0f, 0.0f, -3.0f});

		auto projected_view = projection * glm::inverse(view);

		graphics::clear(0.05f, 0.05f, 0.05f, 1.0f);

		polygon_batch->append(polygon_points);
		polygon_batch->draw(projected_view);

		line_batch->append(line_points);
		line_batch->draw(projected_view);

		point_batch->append(points);
		point_batch->draw(projected_view);

		glfwSwapBuffers(window);
	}

	delete polygon_batch;
	delete line_batch;
	delete point_batch;

	graphics::shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
