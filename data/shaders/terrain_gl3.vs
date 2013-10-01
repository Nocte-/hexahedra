#version 130
#extension GL_EXT_gpu_shader4 : enable

uniform vec3 amb_color;
uniform vec3 sun_color;
uniform vec3 art_color;
 
in vec3  position;
in vec2  uv;
in int   texture;
in ivec2 data;

out vec3 ex_TexCoord;
out vec3 ex_Light;

void main()
{
    ex_Light = clamp(  amb_color * pow((data[1] / 16  ) / 15.0, 0.7)
                     + sun_color * pow((data[1] & 0x0f) / 15.0, 0.7)
                     + art_color * pow((data[0] & 0x0f) / 15.0, 0.7)
                     + 0.03                                          , 0, 1);

    ex_TexCoord = vec3(uv.x / 16.f, uv.y / 16.f, texture); 

	gl_Position = gl_ModelViewProjectionMatrix * vec4(position,1.0);
}

