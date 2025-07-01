#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <span>
#include <algorithm>

#include <glad/gles2.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace glent::graphics {

struct Point {
	glm::vec3 position;
	float size;
	glm::vec4 color;
};

struct VertexAttribute {
	GLuint index;
	GLenum type;
	GLint components;
	bool normalized;
};

using VertexLayout = std::span<const VertexAttribute>;

struct PrimitiveState {
	GLenum mode;
	GLenum cull_mode = GL_BACK;
	GLenum front_face = GL_CCW;
};

struct DepthStencilState {
	bool depth_write;
	GLenum depth_compare = GL_LESS;
	// TODO: Depth bias
	// TODO: Stencil configuration
};

struct BlendState {
	bool enable;
	GLenum color_src_factor = GL_ONE;
	GLenum color_dst_factor = GL_ZERO;
	GLenum color_operation = GL_FUNC_ADD;
	GLenum alpha_src_factor = GL_ONE;
	GLenum alpha_dst_factor = GL_ZERO;
	GLenum alpha_operation = GL_FUNC_ADD;
};

class Buffer final {
public:
	Buffer(GLenum type, GLenum usage, size_t size,
	       const void* data = nullptr);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) noexcept = delete;

	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) noexcept = delete;

	void assign(size_t size, const void* data, uintptr_t offset = 0);

	GLenum type() const noexcept { return type_; }
	GLuint handle() const noexcept { return handle_; }

private:
	GLenum type_;
	GLenum usage_;
	size_t size_;

	GLuint handle_;
};

class Shader final {
public:
	Shader(GLenum type, const std::string_view source);
	~Shader();

	Shader(const Shader&) = delete;
	Shader(Shader&&) noexcept = delete;

	Shader& operator=(const Shader&) = delete;
	Shader& operator=(Shader&&) noexcept = delete;

	GLenum type() const noexcept { return type_; }
	GLenum handle() const noexcept { return handle_; }

private:
	GLenum type_;

	GLenum handle_;
};

class Texture final {
public:
	Texture(GLenum format, uint32_t width, uint32_t height,
	        const void* data = nullptr);
	~Texture();

	Texture(const Texture&) = delete;
	Texture(Texture&&) noexcept = delete;

	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&&) noexcept = delete;

	GLenum type() const noexcept { return type_; }
	GLuint handle() const noexcept { return handle_; }

private:
	GLenum type_;
	GLenum format_;
	
	GLuint handle_;
};

class Sampler final {
public:
	struct Descriptor {
		GLenum address_mode_u = GL_CLAMP_TO_EDGE;
		GLenum address_mode_v = GL_CLAMP_TO_EDGE;
		GLenum address_mode_w = GL_CLAMP_TO_EDGE;
		GLenum min_filter = GL_NEAREST;
		GLenum mag_filter = GL_LINEAR;
		GLenum min_lod = -1000;
		GLenum max_lod = 1000;
		GLenum compare_func = GL_NEVER;
	};

public:
	explicit Sampler(const Descriptor&);
	~Sampler();

	Sampler(const Sampler&) = delete;
	Sampler(Sampler&&) noexcept = delete;

	Sampler& operator=(const Sampler&) = delete;
	Sampler& operator=(Sampler&&) noexcept = delete;

	GLuint handle() const noexcept { return handle_; }

private:
	GLuint handle_;
};

class Pipeline final {
public:
	Pipeline(const PrimitiveState& primitive,
	         const VertexLayout layout,
	         const Shader& vertex_shader,
	         const Shader& fragment_shader,
	         const DepthStencilState& depth_stencil,
	         const BlendState& blend);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&) noexcept = delete;

	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&) noexcept = delete;

	const PrimitiveState& primitiveState() const & noexcept { return primitive_state_; }
	const DepthStencilState& depthStencilState() const & noexcept { return depth_stencil_state_; }
	const BlendState& blendState() const & noexcept { return blend_state_; }
	GLintptr vertexStride() const noexcept { return vertex_stride_; }

	GLuint vertexArray() const noexcept { return vertex_array_; }
	GLuint program() const noexcept { return program_; }

private:
	const PrimitiveState primitive_state_;
	const DepthStencilState depth_stencil_state_;
	const BlendState blend_state_;
	GLintptr vertex_stride_ = 0;

	GLuint vertex_array_;
	GLuint program_;
};

class PointBatch final {
public:
	explicit PointBatch(size_t capacity)
	: capacity_{capacity},
	  points_(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW,
	          capacity * sizeof(Point)) {}
	~PointBatch() = default;

	PointBatch(const PointBatch&) = delete;
	PointBatch(PointBatch&&) noexcept = delete;

	PointBatch& operator=(const PointBatch&) = delete;
	PointBatch& operator=(PointBatch&&) noexcept = delete;

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

class LineBatch final {
public:
	explicit LineBatch(size_t capacity)
	: capacity_{capacity},
	  points_(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW,
	          2 * capacity * sizeof(Point)) {}
	~LineBatch() = default;

	LineBatch(const LineBatch&) = delete;
	LineBatch(LineBatch&&) noexcept = delete;

	LineBatch& operator=(const LineBatch&) = delete;
	LineBatch& operator=(LineBatch&&) noexcept = delete;

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

void setup(uint32_t width, uint32_t height);
void shutdown();

void clear(float red, float green, float blue, float alpha);

void setPipeline(const Pipeline&);
void setVertexBuffer(const Buffer&);
void setIndexBuffer(const Buffer&, GLenum index_type);
void setUniformBuffer(const Buffer&, uint32_t binding);
void setStorageBuffer(const Buffer&, uint32_t binding);
void setTexture(const Texture&, const Sampler&, uint32_t binding);

void draw(uint32_t count, uint32_t offset = 0);
void drawInstanced(uint32_t instances, uint32_t count, uint32_t offset = 0);

} // namespace glent::graphics
