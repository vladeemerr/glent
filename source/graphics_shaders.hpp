constexpr char untextured_unlit_vertex_shader_code[] = R"(
#version 310 es

layout(location = 0) in vec3 v_position;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec4 color;
};

void main() {
	gl_Position = view_projection * transform * vec4(v_position, 1.0f);
}
)";

constexpr char untextured_unlit_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

out vec4 frag_color;

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec4 color;
};

void main() {
	frag_color = color;
}
)";

constexpr char untextured_lit_vertex_shader_code[] = R"(
#version 310 es

struct Light {
	mediump vec4 position_size;
	mediump vec3 color;
};

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

out vec3 f_position;
out vec3 f_normal;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
	vec3 ambience;
	int light_count;
	Light lights[16];
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec3 albedo_color;
	vec3 specular_color;
	float shininess;
	float emissiveness;
};

void main() {
	vec4 position = transform * vec4(v_position, 1.0f);
	vec4 normal = transform * vec4(v_normal, 0.0f);

	gl_Position = view_projection * position;
	f_position = position.xyz;
	f_normal = normalize(normal.xyz);
}
)";

constexpr char untextured_lit_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

struct Light {
	vec4 position_size;
	vec3 color;
};

in vec3 f_position;
in vec3 f_normal;

out vec4 frag_color;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
	vec3 ambience;
	int light_count;
	Light lights[16];
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec3 albedo_color;
	vec3 specular_color;
	float shininess;
	float emissiveness;
};

void main() {
	vec3 normal = normalize(f_normal);
	vec3 view_direction = normalize(view_position - f_position);

	vec3 color = ambience * albedo_color;
	for (int i = 0; i < light_count; ++i) {
		Light light = lights[i];

		vec3 light_direction = normalize(light.position_size.xyz - f_position);
		vec3 half_vector = normalize(view_direction + light_direction);
		float light_distance = distance(light.position_size.xyz, f_position);

		float coeff = max(dot(normal, light_direction), 0.0f);
		float falloff = (light.position_size.w * light.position_size.w) /
		                (1.0f + light_distance * light_distance);

		vec3 specular = pow(max(dot(normal, half_vector), 0.0f), shininess) *
		                specular_color;

		color += (specular + albedo_color) * coeff * light.color * falloff;
	}

	color += albedo_color * emissiveness;

	frag_color = vec4(color, 1.0f);
}
)";

constexpr char textured_lit_vertex_shader_code[] = R"(
#version 310 es

struct Light {
	mediump vec4 position_size;
	mediump vec3 color;
};

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

out vec3 f_position;
out vec3 f_normal;
out vec2 f_uv;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
	vec3 ambience;
	int light_count;
	Light lights[16];
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec3 albedo_color;
	vec3 specular_color;
	float shininess;
	float emissiveness;
};

void main() {
	vec4 position = transform * vec4(v_position, 1.0f);
	vec4 normal = transform * vec4(v_normal, 0.0f);

	gl_Position = view_projection * position;
	f_position = position.xyz;
	f_normal = normalize(normal.xyz);
	f_uv = v_uv;
}
)";

constexpr char textured_lit_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

struct Light {
	vec4 position_size;
	vec3 color;
};

in vec3 f_position;
in vec3 f_normal;
in vec2 f_uv;

out vec4 frag_color;

layout(binding = 0) uniform sampler2D albedo_texture;

layout(std140, binding = 0) uniform CameraUniforms {
	mat4 view_projection;
	vec3 view_position;
	vec3 ambience;
	int light_count;
	Light lights[16];
};

layout(std140, binding = 1) uniform ModelUniforms {
	mat4 transform;
	vec3 albedo_color;
	vec3 specular_color;
	float shininess;
	float emissiveness;
};

void main() {
	vec3 normal = normalize(f_normal);
	vec3 view_direction = normalize(view_position - f_position);

	vec3 albedo = texture(albedo_texture, f_uv).rgb * albedo_color;

	vec3 color = ambience * albedo;
	for (int i = 0; i < light_count; ++i) {
		Light light = lights[i];

		vec3 light_direction = normalize(light.position_size.xyz - f_position);
		vec3 half_vector = normalize(view_direction + light_direction);
		float light_distance = distance(light.position_size.xyz, f_position);

		float coeff = max(dot(normal, light_direction), 0.0f);
		float falloff = (light.position_size.w * light.position_size.w) /
		                (1.0f + light_distance * light_distance);

		vec3 specular = pow(max(dot(normal, half_vector), 0.0f), shininess) *
		                specular_color;

		color += (specular + albedo) * coeff * light.color * falloff;
	}

	color += albedo * emissiveness;

	frag_color = vec4(color, 1.0f);
}
)";

constexpr char sky_vertex_shader_code[] = R"(
#version 310 es

layout(location = 0) in vec2 v_position;

out vec3 f_direction;

layout(std140, binding = 0) uniform SkyUniforms {
	mat4 view;
	vec2 viewport;
};

void main() {
	vec2 uv = vec2(-v_position.x, -v_position.y * viewport.y / viewport.x);
	vec4 position = view * vec4(uv, 1.0f, 1.0f);

	gl_Position = vec4(v_position, 0.0f, 1.0f);
	f_direction = normalize(position.xyz);
}
)";

constexpr char sky_fragment_shader_code[] = R"(
#version 310 es
precision mediump float;

in vec3 f_direction;

out vec4 frag_color;

void main() {
	const vec3 above = vec3(0.52f, 0.81f, 0.92f);
	const vec3 below = vec3(0.05f, 0.05f, 0.05f);
	frag_color = vec4(mix(above, below, smoothstep(0.95f, 1.05f, f_direction.y + 1.0f)), 1.0f);
}
)";
