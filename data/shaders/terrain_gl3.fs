#version 130
#extension GL_EXT_texture_array : enable

uniform sampler2DArray tex; 
uniform vec3 fog_color;
uniform float fog_density;

in vec3 ex_TexCoord;
in vec3 ex_Light;
out vec4 fragColor;

vec4 fog(vec4 color, float depth)
{
    const float e = 2.718281828;
    float f = pow(e, -pow(depth * fog_density, 2));
    return mix(vec4(fog_color, 1.0), color, f);
}

void main() 
{ 
    vec4 color = texture(tex, ex_TexCoord) * vec4(ex_Light, 1.0);
    float z = 1.0 - (gl_FragCoord.z / gl_FragCoord.w);
    fragColor = fog(color, z);
}

