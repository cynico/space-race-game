#version 330 core

out vec4 frag_color;

// In this shader, we want to draw a checkboard where the size of each tile is (size x size).
// The color of the bottom-left most tile should be "colors[0]" and the 2 tiles adjacent to it
// should have the color "colors[1]".

// DONE: (Req 1) Finish this shader.

uniform int size = 32;
uniform vec3 colors[2];

void main(){

    if (
        mod(mod(int(gl_FragCoord.x / size) , 2.0) + mod( int(gl_FragCoord.y / size), 2.0), 2.0)  == 0.0
    ) {
        gl_FragColor = vec4(colors[0], 1.0);
    } else {
        gl_FragColor = vec4(colors[1], 1.0);
    }
}