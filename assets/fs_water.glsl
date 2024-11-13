#version 300 es
precision mediump float;

out vec4 f_color;

uniform vec4 u_color;

void main() {
    f_color = u_color;
}
