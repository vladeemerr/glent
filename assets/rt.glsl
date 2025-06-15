@shader:compute
#version 310 es
precision highp image2D;

struct Vertex {
	vec3 position;
	vec3 color;
};

layout(local_size_x = 8, local_size_y = 8) in;

layout(rgba32f, binding = 0) uniform writeonly image2D image;

layout(std430, binding = 1) readonly buffer storage {
	Vertex vertices[];
};

layout(std140, binding = 2) uniform variables {
	vec3 origin;
};

vec4 rayTriIntersect(in vec3 origin, in vec3 direction,
                     in vec3 v0, in vec3 v1, in vec3 v2) {
	vec3 e1 = v1 - v0;
	vec3 e2 = v2 - v0;

	vec3 p = cross(direction, e2);
	float det = dot(e1, p);

	if (abs(det) < 1e-6f) {
		return vec4(0.0f, 0.0f, 0.0f, -1.0f);
	}

	float inv_det = 1.0f / det;
	vec3 t = origin - v0;

	float u = inv_det * dot(t, p);
	if (u < 0.0f || u > 1.0f) {
		return vec4(0.0f, 0.0f, 0.0f, -1.0f);
	}

	vec3 q = cross(t, e1);
	float v = inv_det * dot(direction, q);
	if (v < 0.0f || u + v > 1.0f) {
		return vec4(0.0f, 0.0f, 0.0f, -1.0f);
	}

	float d = inv_det * dot(e2, q);

	return vec4(u, v, 1.0f - u - v, d);
}

void main() {
	ivec2 p = ivec2(gl_GlobalInvocationID.xy);
	vec2 size = vec2(imageSize(image));

	vec2 uv0 = vec2(1.0f, size.y / size.x);
	vec2 uv = 2.0f * (vec2(p) / size.x) - uv0;

	vec3 d = vec3(uv, 1.0f);
	vec4 hit = rayTriIntersect(origin, d,
	                           vertices[0].position,
	                           vertices[1].position,
	                           vertices[2].position);
	
	vec4 color = vec4(0.05f, 0.05f, 0.05f, 1.0f);
	if (hit.w > 0.0f) {
		color.rgb = hit.z * vertices[0].color +
		            hit.x * vertices[1].color +
		            hit.y * vertices[2].color;
	}

	imageStore(image, p, color);
}
