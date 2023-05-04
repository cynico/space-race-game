#version 330

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;


// VP is the view and projection matrix multiplied.
uniform mat4 VP;

// The camera position.
uniform vec3 camera_position;

// The model matrix.
uniform mat4 M;

// The inverse of the model matrix transposed.
// This could've been calculated here, in the GPU, from the above M
// but it's more computationally expensive, given that it'll
// be calculated for every vertex (the vertex shader is called for every vertex).
uniform mat4 M_IT;

out Varyings {
    vec2 tex_coord;
    vec3 normal;
    vec3 view;
    vec3 world;
} vs_out;

void main() {

    // Calculating the world position of the vertex by multiplying
    // the position with the world matrix. This is used in the fragment 
    // shader.
    vec3 world = (M * vec4(position, 1.0)).xyz;
    vs_out.world = world;

    // Setting the position of the vertex, by multiplying the world
    // position with the VP matrices.
    gl_Position = VP * vec4(world, 1.0);

    // Setting the texture coordinate of the fragment.
    vs_out.tex_coord = tex_coord;

    // Setting the normal vector to the surface at the fragment's position.
    // It's calculated as : normalize(Transpose(Inverse(World Matrix)) * NormalVector)
    vs_out.normal = normalize((M_IT * vec4(normal, 0.0)).xyz);

    // The view vector is the direction from the camera position to
    // the fragment's position in the world.
    vs_out.view = camera_position - world;
}