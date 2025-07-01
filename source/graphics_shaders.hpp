constexpr char point_batch_vs_source[] = R"(
#version 310 es

struct Point {
	vec4 position_size;
	vec4 color;
};

layout(location = 0) in vec4 in_position;

flat out vec4 out_color;
out vec2 out_uv;
out flat float out_size;

layout(std140, binding = 0) uniform Uniforms {
	mat4 projected_view;
	vec2 one_over_viewport;
};

layout(std430, binding = 1) readonly buffer Instances {
	Point points[];
};

void main() {
	Point point = points[gl_InstanceID];

	vec4 position = projected_view * vec4(point.position_size.xyz, 1.0f);
	vec2 size = point.position_size.w * one_over_viewport;
	position.xy += in_position.xy * size * position.w;

	gl_Position = position;
	out_color = point.color;
	out_uv = in_position.xy * point.position_size.w;
	out_size = point.position_size.w + 0.5f;
}
)";

constexpr char point_batch_fs_source[] = R"(
#version 310 es
precision mediump float;

flat in vec4 out_color;
in vec2 out_uv;
flat in float out_size;

out vec4 frag_color;

void main() {
	vec4 color = out_color;
	color.a *= out_size - length(out_uv);

	if (color.a <= 0.0f)
		discard;

	frag_color = color;
}
)";

constexpr char line_batch_vs_source[] = R"(
#version 310 es

struct Point {
	vec4 position_size;
	vec4 color;
};

layout(location = 0) in vec4 in_position;

out vec4 out_color;

layout(std140, binding = 0) uniform Uniforms {
	mat4 projected_view;
	vec2 one_over_viewport;
};

layout(std430, binding = 1) readonly buffer Instances {
	Point points[];
};

void main() {
	Point point = points[2 * gl_InstanceID + (gl_VertexID & 1)];

	vec4 positions[2];
	positions[0] = projected_view * vec4(points[2 * gl_InstanceID].position_size.xyz, 1.0f);
	positions[1] = projected_view * vec4(points[2 * gl_InstanceID + 1].position_size.xyz, 1.0f);

	vec2 dir = normalize((positions[0].xy / positions[0].w) -
	                     (positions[1].xy / positions[1].w));
	vec2 perp = vec2(-dir.y, dir.x);

	vec4 position = positions[gl_VertexID & 1];
	position.xy += perp * in_position.y * position.w *
	               point.position_size.w * one_over_viewport;

	gl_Position = position;
	out_color = point.color;
}
)";

constexpr char line_batch_fs_source[] = R"(
#version 310 es
precision mediump float;

in vec4 out_color;

out vec4 frag_color;

void main() {
	frag_color = out_color;
}
)";
