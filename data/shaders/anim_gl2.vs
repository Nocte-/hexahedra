#version 120

attribute vec3 pos;
attribute vec2 tex_coord;
attribute vec3 normal;
attribute vec4 weights;
attribute vec4 bones;

uniform mat3x4 bonemats[80];

void main(void)
{
    mat3x4 m = bonemats[int(bones.x)] * weights.x;
    m += bonemats[int(bones.y)] * weights.y;
    m += bonemats[int(bones.z)] * weights.z;
    m += bonemats[int(bones.w)] * weights.w;

    mat3 adj = mat3(cross(m[1].xyz, m[2].xyz), cross(m[2].xyz, m[0].xyz), cross(m[0].xyz, m[1].xyz));
    normal *= adj;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos * m, 1);
    gl_TexCoord[0] = vec4(uv, 1, 1);
    gl_FrontColor = gl_Color * (clamp(dot(normalize(gl_NormalMatrix * normal), gl_LightSource[0].position.xyz), 0.0, 1.0) * gl_LightSource[0].diffuse + gl_LightSource[0].ambient);
}

