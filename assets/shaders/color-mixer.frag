#version 330 core

// This shader is designed to work with "triangle.vert" and it receives an
// interpolated varying which represents the vertex color.

in Varyings {
    vec3 color;
} fs_in;

out vec4 frag_color;

uniform vec4 red;
uniform vec4 green;
uniform vec4 blue;

// currently the shader just returns the interpalated color varying.
// However, we want to mix the color channels around. We can do this using a 
// color matrix which we will send to the shader as 3 uniforms: red, green, blue.
// Each of these 3 variables will be a vec4. To apply the channel mixing for a
// certain channel (e.g. red), we apply this linear transformation:
// frag_color.r = red.r * fs_in.color.r + red.g * fs_in.color.g + red.b * fs_in.color.b + red.a; => CORRECTION
// However, this line is too long to write, so we can simplify it using a dot product
// (which is defined in the "dot" function).

// DONE: (Req 1) Finish this shader and apply the channel mixing using the "dot" function.

void main(){

    // If the three uniforms are zeroed out, then they aren't defined.
    // Use color are is.
    if (
        red == vec4(0.0, 0.0, 0.0, 0.0) &&
        blue == vec4(0.0, 0.0, 0.0, 0.0) &&
        green == vec4(0.0, 0.0, 0.0, 0.0)
    ) {
        frag_color = vec4(fs_in.color, 1.0);
        return;
    }

    frag_color = vec4(
        dot(red, vec4(fs_in.color, 1.0)), 
        dot(green, vec4(fs_in.color, 1.0)),
        dot(blue, vec4(fs_in.color, 1.0)), 
        1.0
    );

}