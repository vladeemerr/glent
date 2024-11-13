#version 300 es

layout(location = 0) in vec3 v_position;

uniform mat4 u_view_projection;
uniform mat4 u_model;

uniform float u_time;

void main() {
    vec3 position = v_position;
    position.y = sin(u_time + position.x + position.z) * 0.1f;
    gl_Position = u_view_projection * u_model * vec4(position, 1.0f);
}
