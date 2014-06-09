#version 120

uniform mat4  matrix;

attribute vec3  postion;
attribute vec2  uv;
attribute vec3  normal;

varying vec2  frag_uv;
varying float frag_diffuse;

const float light_ambient = 0.1;
const vec3  light_direction = normalize(vec3(1.0, 1.0, 1.0));

void main(void) {
    gl_Position = matrix * vec4(postion,1);
    frag_uv = uv;
    frag_diffuse = min(1.0, 
                       light_ambient + max(0.0, dot(normal, light_direction)));
}

