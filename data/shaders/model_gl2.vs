#version 120

attribute vec3 pos;
attribute vec2 tex_coord;
attribute vec3 normal;

void main(void)
{
    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos,1);
    gl_TexCoord[0] = vec4(tex_coord, 1, 1);
    gl_FrontColor = gl_Color * (clamp(dot(normalize(gl_NormalMatrix * normal), gl_LightSource[0].position.xyz), 0.0, 1.0) * gl_LightSource[0].diffuse + gl_LightSource[0].ambient);
}

