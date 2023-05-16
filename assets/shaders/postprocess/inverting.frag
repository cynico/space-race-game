#version 330

uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
    frag_color = texture(tex, tex_coord);
    frag_color = vec4(1 - frag_color.r, 1 - frag_color.g, 1 - frag_color.b, frag_color.a);
}