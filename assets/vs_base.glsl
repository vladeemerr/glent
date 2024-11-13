#version 300 es

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;

out vec4 f_position;
out vec3 f_normal;
out vec2 f_uv;

uniform mat4 u_view_projection;
uniform mat4 u_model;

void main() {
    f_position = u_model * vec4(v_position, 1.0f);
    gl_Position = u_view_projection * f_position;

    vec4 normal = u_model * vec4(v_normal, 1.0f);
    f_normal = normal.xyz;

    f_uv = v_uv;
}
