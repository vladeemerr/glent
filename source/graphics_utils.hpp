#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

#include "graphics.hpp"

namespace glint::graphics::utils {

struct Point {
	glm::vec3 position;
	float size;
	glm::vec4 color;

	Point() = default;
	Point(glm::vec3 position, float size, glm::vec4 color)
	: position{position}, size{size}, color{color} {}
	Point(glm::vec3 position, glm::vec4 color)
	: position{position}, color{color} {}
};

template<size_t N, GLenum T>
class Batch final {
public:
	explicit Batch(size_t capacity)
	: capacity_{N * capacity},
	  points_(T, GL_DYNAMIC_DRAW, N * capacity * sizeof(Point)) {}
	~Batch() = default;

	Batch(const Batch&) = delete;
	Batch(Batch&&) noexcept = delete;

	Batch& operator=(const Batch&) = delete;
	Batch& operator=(Batch&&) noexcept = delete;

	size_t append(const std::span<const Point> points) {
		size_t count = std::min(points.size(), capacity_ - size_);

		if (count == 0)
			return 0;

		points_.assign(count * sizeof(Point), points.data(),
		               size_ * sizeof(Point));
		size_ += count;

		return count;
	}

	void draw(const glm::mat4& projected_view);

private:
	size_t size_ = 0;
	size_t capacity_;
	Buffer points_;
};

using PointBatch = Batch<1, GL_SHADER_STORAGE_BUFFER>;
using LineBatch = Batch<2, GL_SHADER_STORAGE_BUFFER>;
using PolygonBatch = Batch<3, GL_ARRAY_BUFFER>;

void setup();
void shutdown();

} // namespace glint::graphics::utils
