@shader:vertex

#version 300 es

void main() {
	int vertex = gl_VertexID;
	vec2 position = 2.0f * vec2(vertex & 1, vertex >> 1) - 1.0f;
	gl_Position = vec4(position, 0.0f, 1.0f);
}

@shader:fragment

#version 300 es
precision mediump float;

out vec4 color;

void main() {
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
