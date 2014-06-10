#version 120

uniform sampler2D   tex;
uniform vec3        fog_color;

varying vec2    frag_uv;
varying vec3    frag_color;
varying float   frag_fog;

void main() { 
    vec4 color = vec4(texture2D(tex, frag_uv)) * vec4(frag_color, 1.0);
    gl_FragColor = mix(color, vec4(fog_color, 1.0), frag_fog);
}

