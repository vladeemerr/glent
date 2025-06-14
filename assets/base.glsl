@shader:vertex

#version 300 es

out vec2 uv_out;

void main() {
	int vertex = gl_VertexID;
	vec2 uv = vec2(vertex & 1, vertex >> 1);
	vec2 position = 2.0f * uv - 1.0f;
	uv_out = uv;
	gl_Position = vec4(position, 0.0f, 1.0f);
}

@shader:fragment

#version 300 es
precision mediump float;

in vec2 uv_out;

out vec4 color;

uniform sampler2D texture0;

void main() {
	color = texture(texture0, uv_out);
}
