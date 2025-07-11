#include "graphics.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sstream>

namespace glint::graphics {

namespace {

GLenum current_primitive_mode;
GLintptr current_vertex_stride;
GLenum current_index_type;

uint32_t current_viewport_width;
uint32_t current_viewport_height;

void GLAPIENTRY glDebugCallback(GLenum /*source*/, GLenum type,
                                GLuint /*id*/, GLenum /*severity*/,
                                GLsizei /*length*/, const GLchar* message,
                                const void* /*user_param*/) {
	if (type != GL_DEBUG_TYPE_ERROR_KHR) {
		return;
	}

	std::cerr << "GLES ERROR: " << message << '\n';
}

inline GLsizei sizeFromType(GLenum type) {
	switch (type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return 1;
		case GL_SHORT:
		case GL_HALF_FLOAT:
		case GL_UNSIGNED_SHORT:
			return 2;
		case GL_INT:
		case GL_FLOAT:
		case GL_UNSIGNED_INT:
			return 4;
	}
	
	return 0;
}

inline GLenum formatFromInternalFormat(GLenum format) {
	switch (format) {
		case GL_R8: return GL_RED;
		case GL_RG8: return GL_RG;
		case GL_RGB8: return GL_RGB;
		case GL_RGBA8: return GL_RGBA;
	}

	return 0;
}

inline GLenum typeFromInternalFormat(GLenum format) {
	switch (format) {
		case GL_R8:
		case GL_RG8:
		case GL_RGB8:
		case GL_RGBA8:
			return GL_UNSIGNED_BYTE;
	}

	return 0;
}

} // namespace

Buffer::Buffer(GLenum type, GLenum usage, size_t size, const void* data)
: type_{type}, usage_{usage}, size_{size} {
	assert(type == GL_ARRAY_BUFFER ||
	       type == GL_ELEMENT_ARRAY_BUFFER ||
	       type == GL_UNIFORM_BUFFER ||
	       type == GL_SHADER_STORAGE_BUFFER);
	assert(usage == GL_STATIC_DRAW || usage == GL_DYNAMIC_DRAW);
	assert(size != 0);
	assert(usage != GL_STATIC_DRAW || data != nullptr);
	
	glGenBuffers(1, &handle_);
	glBindBuffer(type, handle_);

	glBufferData(type, size, data, usage);

	glBindBuffer(type, 0);
}

Buffer::~Buffer() {
	glDeleteBuffers(1, &handle_);
}

void Buffer::assign(size_t size, const void* data, uintptr_t offset) {
	assert(size != 0 && data != nullptr && size <= size_ - offset);

	glBindBuffer(type_, handle_);
	glBufferSubData(type_, offset, size, data);
	glBindBuffer(type_, 0);
}

Shader::Shader(GLenum type, const std::string_view source) {
	handle_ = glCreateShader(type);

	const char* cstr = source.data();
	const GLint length = source.length();

	glShaderSource(handle_, 1, &cstr, &length);
	glCompileShader(handle_);

	GLint success = 0;
	glGetShaderiv(handle_, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint log_length;
		glGetShaderiv(handle_, GL_INFO_LOG_LENGTH, &log_length);

		char* log = static_cast<char*>(alloca(log_length));
		glGetShaderInfoLog(handle_, log_length, nullptr, log);

		std::stringstream ss;
		switch (type) {
			case GL_VERTEX_SHADER:
				ss << "Vertex";
				break;
			case GL_FRAGMENT_SHADER:
				ss << "Fragment";
				break;
			default:
				ss << "Unknown";
		}

		ss << " shader compilation error:\n" << log << '\n';
		throw std::runtime_error(ss.str());
	}
}

Shader::~Shader() {
	glDeleteShader(handle_);
}

Texture::Texture(GLenum format, uint32_t width, uint32_t height, const void* data)
: type_{GL_TEXTURE_2D}, format_{format} {
	assert(width != 0 && height != 0);
	
	glGenTextures(1, &handle_);
	glBindTexture(GL_TEXTURE_2D, handle_);

	glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

	if (data != nullptr) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
		                formatFromInternalFormat(format),
		                typeFromInternalFormat(format),
		                data);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &handle_);
}

Sampler::Sampler(const Descriptor& descriptor) {
	glGenSamplers(1, &handle_);

	glSamplerParameteri(handle_, GL_TEXTURE_WRAP_S, descriptor.address_mode_u);
	glSamplerParameteri(handle_, GL_TEXTURE_WRAP_T, descriptor.address_mode_v);
	glSamplerParameteri(handle_, GL_TEXTURE_WRAP_R, descriptor.address_mode_w);
	glSamplerParameteri(handle_, GL_TEXTURE_MIN_FILTER, descriptor.min_filter);
	glSamplerParameteri(handle_, GL_TEXTURE_MAG_FILTER, descriptor.mag_filter);
	glSamplerParameteri(handle_, GL_TEXTURE_MIN_LOD, descriptor.min_lod);
	glSamplerParameteri(handle_, GL_TEXTURE_MAX_LOD, descriptor.max_lod);
	glSamplerParameteri(handle_, GL_TEXTURE_COMPARE_FUNC, descriptor.compare_func);
}

Sampler::~Sampler() {
	glDeleteSamplers(1, &handle_);
}

Pipeline::Pipeline(const PrimitiveState& primitive,
                   const VertexLayout layout,
                   const Shader& vertex_shader,
                   const Shader& fragment_shader,
                   const DepthStencilState& depth_stencil,
                   const BlendState& blend_state)
: primitive_state_{primitive},
  depth_stencil_state_{depth_stencil},
  blend_state_{blend_state} {
	glGenVertexArrays(1, &vertex_array_);
	glBindVertexArray(vertex_array_);

	for (const auto& attrib : layout) {
		vertex_stride_ += attrib.components * sizeFromType(attrib.type);
	}

	GLuint offset = 0;
	for (const auto& attrib : layout) {
		glEnableVertexAttribArray(attrib.index);
		if (attrib.type == GL_FLOAT ||
		    attrib.type == GL_HALF_FLOAT ||
		    attrib.normalized) {
			glVertexAttribFormat(attrib.index, attrib.components, attrib.type,
			                     attrib.normalized, offset);
		} else {
			glVertexAttribIFormat(attrib.index, attrib.components, attrib.type, offset);
		}
		glVertexAttribBinding(attrib.index, 0);

		offset += attrib.components * sizeFromType(attrib.type);
	}

	glBindVertexArray(0);

	program_ = glCreateProgram();

	glAttachShader(program_, vertex_shader.handle());
	glAttachShader(program_, fragment_shader.handle());
	glLinkProgram(program_);

	GLint link_success;
	glGetProgramiv(program_, GL_LINK_STATUS, &link_success);
	if (!link_success) {
		GLint log_length;
		glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);

		char* log = static_cast<char*>(alloca(log_length));
		glGetProgramInfoLog(program_, log_length, nullptr, log);

		std::stringstream ss;
		ss << "Shader program linking error:\n" << log << '\n';
		throw std::runtime_error(ss.str());
	}

	glDetachShader(program_, fragment_shader.handle());
	glDetachShader(program_, vertex_shader.handle());
}

Pipeline::~Pipeline() {
	glDeleteProgram(program_);
	glDeleteVertexArrays(1, &vertex_array_);
}

void setup(uint32_t width, uint32_t height) {
	glEnable(GL_DEBUG_OUTPUT_KHR);
	glDebugMessageCallbackKHR(glDebugCallback, 0);

	glViewport(0, 0, width, height);

	current_viewport_width = width;
	current_viewport_height = height;
}

void shutdown() {}

glm::vec2 viewport() {
	return {current_viewport_width, current_viewport_height};
}

void clear(float red, float green, float blue, float alpha) {
	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void setPipeline(const Pipeline& pipeline) {
	const auto& primitive = pipeline.primitiveState();
	const auto& depth_stencil = pipeline.depthStencilState();
	const auto& blend = pipeline.blendState();
	
	current_primitive_mode = primitive.mode;
	current_vertex_stride = pipeline.vertexStride();
	current_index_type = GL_NONE;

	if (primitive.cull_mode != GL_NONE) {
		glEnable(GL_CULL_FACE);
		glCullFace(primitive.cull_mode);
		glFrontFace(primitive.front_face);
	} else {
		glDisable(GL_CULL_FACE);
	}

	if (depth_stencil.depth_write) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(depth_stencil.depth_compare);
	} else {
		glDisable(GL_DEPTH_TEST);
	}

	if (blend.enable) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(blend.color_src_factor, blend.color_dst_factor,
		                    blend.alpha_src_factor, blend.alpha_dst_factor);
		glBlendEquationSeparate(blend.color_operation, blend.alpha_operation);
	} else {
		glDisable(GL_BLEND);
	}

	glUseProgram(pipeline.program());
	glBindVertexArray(pipeline.vertexArray());
}

void setVertexBuffer(const Buffer& buffer) {
	assert(buffer.type() == GL_ARRAY_BUFFER);
	glBindVertexBuffer(0, buffer.handle(), 0, current_vertex_stride);
}

void setIndexBuffer(const Buffer& buffer, GLenum index_type) {
	assert(buffer.type() == GL_ELEMENT_ARRAY_BUFFER);
	assert(index_type == GL_UNSIGNED_SHORT || index_type == GL_UNSIGNED_INT);

	current_index_type = index_type;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.handle());
}

void setUniformBuffer(const Buffer& buffer, uint32_t binding) {
	assert(buffer.type() == GL_UNIFORM_BUFFER);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer.handle());
}

void setStorageBuffer(const Buffer& buffer, uint32_t binding) {
	assert(buffer.type() == GL_SHADER_STORAGE_BUFFER);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer.handle());
}

void setTexture(const Texture& texture, const Sampler& sampler, uint32_t binding) {
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindSampler(binding, sampler.handle());
	glBindTexture(texture.type(), texture.handle());
}

void draw(uint32_t count, uint32_t offset) {
	if (current_index_type != GL_NONE) {
		glDrawElements(current_primitive_mode, count, current_index_type,
		               reinterpret_cast<const void*>(offset * sizeFromType(current_index_type)));
	} else {
		glDrawArrays(current_primitive_mode, offset, count);
	}
}

void drawInstanced(uint32_t instances, uint32_t count, uint32_t offset) {
	if (current_index_type != GL_NONE) {
		glDrawElementsInstanced(current_primitive_mode, count, current_index_type,
		                        reinterpret_cast<const void*>(offset * sizeFromType(current_index_type)),
		                        instances);
	} else {
		glDrawArraysInstanced(current_primitive_mode, offset, count, instances);
	}
}

} // namespace glint::graphics
