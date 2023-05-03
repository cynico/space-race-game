#version 330 core

/*
    This is a vertex shader for rendering text onto the screen.
    It takes one input variable, which is the vec4 vertex, consisting of: x and y position of vertex, x and y coordinates of texture.
    It also takes a uniform (orthographic) projection matrix.
    It outputs the texture coordinates for the fragment shader.
*/

layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 projection;

void main() {
    
    // Here, we get the position (z component is not varied, thus we only need x and y), and multiply it by the provided projection matrix.
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    
    // Output the texture coordinates to the fragment shader. 
    TexCoords = vertex.zw;
}