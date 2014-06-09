#version 130
#extension GL_EXT_texture_array : enable

uniform sampler2DArray tex; 
uniform vec3 fog_color;

in vec3 frag_uvw;
in vec3 frag_light;
in float frag_fog;
out vec4 fragColor;

void main() { 
    vec4 color = vec4(texture(tex, frag_uvw)) * vec4(frag_light, 1.0);
    fragColor = mix(color, vec4(fog_color, 1.0), frag_fog);
}

