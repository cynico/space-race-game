#version 330

// The texture holding the scene pixels
uniform sampler2D tex;

// Read "assets/shaders/fullscreen.vert" to know what "tex_coord" holds;
in vec2 tex_coord;

out vec4 frag_color;

// Vignette is a postprocessing effect that darkens the corners of the screen
// to grab the attention of the viewer towards the center of the screen

void main(){
    //DONE: (Req 11) Modify this shader to apply vignette
    // To apply vignette, divide the scene color
    // by 1 + the squared length of the 2D pixel location the NDC space
    // Hint: remember that the NDC space ranges from -1 to 1
    // while the texture coordinate space ranges from 0 to 1
    // We have the pixel's texture coordinate, how can we compute its location in the NDC space?
    
    // Apply equation: y = 2x - 1, on each coordinate, where y is the new coordinate value in NDC, and x is the coordinate value
    // in the texture coordinates.
    vec2 ndc_coords = vec2((2 * tex_coord.x) - 1, (2*tex_coord.y) - 1);
    float squaredLength = pow(ndc_coords.x,2) + pow(ndc_coords.y, 2);
    frag_color = texture(tex, tex_coord);

    frag_color = frag_color / (1 + squaredLength);
}