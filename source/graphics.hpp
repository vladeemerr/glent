#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <span>

#include <glad/gles2.h>

namespace glent::graphics {

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

class Pipeline final {
public:
	Pipeline(const PrimitiveState& primitive,
	         const VertexLayout layout,
	         const Shader& vertex_shader,
	         const Shader& fragment_shader,
	         const DepthStencilState& depth_stencil);
	~Pipeline();

	Pipeline(const Pipeline&) = delete;
	Pipeline(Pipeline&&) noexcept = delete;

	Pipeline& operator=(const Pipeline&) = delete;
	Pipeline& operator=(Pipeline&&) noexcept = delete;

	const PrimitiveState& primitiveState() const & noexcept { return primitive_state_; }
	const DepthStencilState& depthStencilState() const & noexcept { return depth_stencil_state_; }
	GLintptr vertexStride() const noexcept { return vertex_stride_; }

	GLuint vertexArray() const noexcept { return vertex_array_; }
	GLuint program() const noexcept { return program_; }

private:
	const PrimitiveState primitive_state_;
	const DepthStencilState depth_stencil_state_;
	GLintptr vertex_stride_ = 0;

	GLuint vertex_array_;
	GLuint program_;
};

void setup(uint32_t width, uint32_t height);
void shutdown();

void clear(float red, float green, float blue, float alpha);

void setPipeline(const Pipeline&);
void setVertexBuffer(const Buffer&);
void setIndexBuffer(const Buffer&, GLenum index_type);
void setUniformBuffer(const Buffer&, uint32_t binding);

void draw(uint32_t count, uint32_t offset = 0);

} // namespace glent::graphics
