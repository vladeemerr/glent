#version 300 es
precision mediump float;

in vec4 f_position;
in vec3 f_normal;
in vec2 f_uv;

out vec4 f_color;

uniform vec4 u_tint;
uniform sampler2D u_texture;

uniform vec3 u_view_position;

void main() {
    const float ambient_factor = 0.1f;
    const float specular_intensity = 1.0f;
    const vec3 light_position = vec3(-1.0f, 3.0f, -1.0f);
    const vec3 light_color = vec3(1.0f, 1.0f, 1.0f);

    const float light_constant_term = 1.0f;
    const float light_linear_term = 0.1f;
    const float light_quadratic_term = 0.03f;

    vec3 normal = normalize(f_normal);
    vec3 position = f_position.xyz;
    vec3 light_direction = normalize(light_position - position);
    float light_distance = length(light_position - position);
    vec3 view_direction = normalize(u_view_position - position);
    vec3 reflection_direction = reflect(-light_direction, normal);

    float diffuse_factor = max(0.0f, dot(normal, light_direction));
    float specular_factor = pow(max(0.0f, dot(view_direction, reflection_direction)), 32.0f);
    float attenuation = 1.0f / (light_constant_term +
                light_linear_term * light_distance +
                light_quadratic_term * light_distance * light_distance);

    vec4 albedo = texture(u_texture, f_uv) * u_tint;
    vec3 ambience = light_color * ambient_factor * attenuation;
    vec3 diffusion = light_color * diffuse_factor * attenuation;
    vec3 specularity = light_color * specular_factor * specular_intensity * attenuation;

    vec4 color = albedo * vec4(ambience + diffusion + specularity, 1.0f);

    if (color.a <= 0.0f)
        discard;

    f_color = color;
}
