#version 120

uniform sampler2D tex;

varying vec2    frag_uv;
varying float   frag_diffuse;

void main(void) {
    gl_FragColor = vec4(vec3(texture2D(tex, frag_uv)) * frag_diffuse, 1.0);
}

