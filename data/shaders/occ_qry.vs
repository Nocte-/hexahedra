#version 120

uniform mat4    matrix;
attribute vec3  postion;

void main(void) {
    gl_Position = matrix * vec4(postion, 1.0);
}

