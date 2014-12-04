#version 130
#extension GL_EXT_gpu_shader4 : enable

uniform mat4 matrix;
uniform vec3 camera;
uniform float fog_distance;
uniform vec3 amb_color;
uniform vec3 sun_color;
uniform vec3 art_color;
 
in      vec3  position;
in      vec2  uv;
flat in int   texture;
in      ivec2 data;

out vec3  frag_uvw;
out vec3  frag_light;
out float frag_fog;

void main() {
    gl_Position = matrix * vec4(position, 1.0);
    frag_uvw = vec3(uv.x / 16.f, uv.y / 16.f, texture);
    frag_light = clamp(
        vec3(0.02, 0.02, 0.02)
      + amb_color * (data[1] / 16) / 15.0
      + sun_color * (data[1] & 0x0f) / 15.0
      + art_color * (data[0] & 0x0f)/ 15.0, 
      0, 1);

    float camera_distance = distance(camera, position);
    frag_fog = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 3.0);
}

