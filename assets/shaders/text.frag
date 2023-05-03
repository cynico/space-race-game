#version 330 core

/*
    This is a fragment shader for rendering text onto the screen.
    It takes the texture (of the character) coordinates from the vertex shader as an input.
    It takes two uniforms:
        - The texture of the character.
        - The color you'd like the text of the color to be.

    It works by varying the alpha component of the rendered text (1.0 for pixels of the character, 0.0 for the background of the character).
    We get that alpha component from the character texture (text).
    This texture is special in that it only has one 8-byte component, not the 4 components of other textures.
    This component is itself the alpha of the pixel, and it's accessed through the red component of the texture.

    This requires blending to be enabled in the corresponding pipeline state.
*/

in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 texColor;

void main() {

    // We're only interested in the alpha component of the current pixel.
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    
    // Multiplying the above by the desired given color outputs: (texColor.r, texColor.g, texColor.b, sampled.a);
    color = vec4(texColor, 1.0) * sampled;
}