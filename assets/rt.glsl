@shader:compute
#version 310 es
precision highp image2D;

layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform writeonly image2D image0;

void main() {
	ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(image0);

	vec2 uv = vec2(p) / vec2(size);
	vec4 color = vec4(uv, 0.0f, 1.0f);

	imageStore(image0, p, color);
}
