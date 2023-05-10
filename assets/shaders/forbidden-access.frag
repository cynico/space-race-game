#version 330

out vec4 frag_color;

void main() {
    // Outputting a transparent red.
    frag_color = vec4(1.0, 0.0, 0.0, 0.2);
}