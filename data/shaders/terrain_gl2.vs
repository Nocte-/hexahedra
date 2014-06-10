#version 120

uniform mat4   matrix;
uniform vec3   camera;
uniform float  fog_distance;

attribute vec3 position;
attribute vec2 uv;
attribute vec3 color;

varying vec2   frag_uv;
varying vec3   frag_color;
varying float  frag_fog;

void main() {
	gl_Position = matrix * vec4(position,1.0);
    frag_uv = uv;
    frag_color = color;

    float camera_distance = distance(camera, position);
    frag_fog = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
}

