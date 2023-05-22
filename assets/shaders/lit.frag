#version 330

#define DIRECTIONAL 0
#define POINT       1
#define SPOT        2

// Definition of light source struct.
// type: DIRECTIONAL, POINT, SPOT
// position: position of the light source, in case of SPOT and POINT 
// direction (direction of light in case of SPOT and DIRECTIONAL)
// color: color of the light source
// attenuation: explained below, at the end of the for loop
// cone_angles: two cone angles associated with SPOT light sources, explained below.
struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 attenuation;
    vec2 cone_angles;
};

// Maximum number of light sources.
#define MAX_LIGHTS 200

// We accept an array of light sources, with maximum
// number of 8 light sources, as a uniform
uniform Light lights[MAX_LIGHTS];

// We accept the number of the light sources in the array.
uniform int light_count;

// Defining a sky with three simulated light source components
// One from above, below, and from the horizon.
struct Sky {
    vec3 top, horizon, bottom;
};

// Accept a sky as a unfiorm.
uniform Sky sky;

// Here, we get a certain ambient light color depending on the position of the pixel.
// This color will affect the ambient color of the pixel at a certain position.
// For example: imagine a ball on the earth, we can reasonably say that the upper pixels will have
// the color of the material, with a hint of blue-ish light component.
// We call this: sky light.
// Sky light has two components depending on the poisiton of the pixel.
// If the normal vector of the surface at the pixel's position is pointing up, we use the sky.top color (blue-ish)
// If it's pointing down, we use something gray, for example.
// Those two are mixed always with a third one, the horizon color. 
// Imagine a light component coming from the horizon.
// The result of mix(a, b, c) is c*a + (1-c)*b  
vec3 compute_sky_light(vec3 normal){
    vec3 extreme = normal.y > 0 ? sky.top : sky.bottom;
    return mix(sky.horizon, extreme, normal.y * normal.y);
}

// This struct holds all the material's textures:
// Albedo (for diffuse and ambient color)
// Specular (for specular color)
// Roughness (for shininess used in computing specular color)
// Ambient Occlusion (for ambient color, explained below in the file)
// Emissive (for the light emitted from the material itself, if it is emissive, like the sun for example).
struct Material {
    sampler2D albedo;
    sampler2D specular;
    sampler2D roughness;
    sampler2D ambient_occlusion;
    sampler2D emissive;
};

// We accept the material used for the current fragment through a uniform.
uniform Material material;

// We accpet input variables from the vertex shader.
// tex_coord is the texture coordinate
// normal is the normal vector the surface at the fragment's position
// view is the camera view position used later for calculating specular component.
// world is the world position of the current fragment.
in Varyings {
    vec2 tex_coord;
    vec3 normal;
    vec3 view;
    vec3 world;
} fs_in;

// We output the final fragment color.
out vec4 frag_color;

// Lambert is used in calculating the diffuse light component.
// It's the dot product of the normal vector and the world_to_light direction
// vector.
float lambert(vec3 normal, vec3 world_to_light_direction) {
    return max(0.0, dot(normal, world_to_light_direction));
}

// This function's calculates the phong's specular component.
// It's the result of the dot product of the reflected light direction
// (around the normal of the surface's pixel), and the view direction (of the camera),
// raised to the power of shininess of the pixel's material.
// We use max() function becausee the result may be negative,
// and we want it to be 0.0 or above.
float phong(vec3 reflected, vec3 view, float shininess) {
    return pow(max(0.0, dot(reflected, view)), shininess);
}

void main() {
    
    // Here, we normalize the input normal vector, and the input
    // camera view vector.
    vec3 normal = normalize(fs_in.normal);
    vec3 view = normalize(fs_in.view);
    
    // We simulate a light component from the sky, that affects the ambient light component
    // of the material.
    vec3 ambient_light = compute_sky_light(normal);

    // Obtaining the material's diffuse color for this pixel from the albedo texture map. 
    vec3 diffuse = texture(material.albedo, fs_in.tex_coord).rgb;

    // Obtaining the material's specular color for this pixel from the specular texture map.
    vec3 specular = texture(material.specular, fs_in.tex_coord).rgb;

    // Obtaining the material's roughness for this pixel from the roughness texture map.
    // Roughness is used later to calculate the shininess, used in computing the final specular light component. 
    float roughness = texture(material.roughness, fs_in.tex_coord).r;

    // To obtain the ambient color, we multiply the diffuse value (obtained from the albedo texture)
    // by the value (only the red component, that is, we're scaling the color) obtained from the ambient
    // occlusion map. This is because ambient light shouldn't reach all locations on the surface on a 
    // similar manner. For example, if you have your hand folded, certain places will be darker than others.
    // We emulate this with the ambient occlusion map. That's where "occlusion" comes from. To occlude is to block.
    vec3 ambient = diffuse * texture(material.ambient_occlusion, fs_in.tex_coord).r;
    
    // Obtaining the material's emissive component for this pixel from the emissive texture map.
    // The emissive texture maps values tell whether a certain pixel of the textuer emits light.
    // If it doesn't, as in the case of non-emissive materials, then the value will be vec3(0.0, 0.0, 0.0) 
    vec3 emissive = texture(material.emissive, fs_in.tex_coord).rgb;

    // Here, we calculate the shineness from the roughness value.
    // The law governing this is: shininess = ( 2 / roughness^4 ) - 2.0
    // Here, we use the function clamp to account for the case where roughness = 0
    // If roughness = 0, the shininess = infinity.
    float shininess = 2.0 / pow(clamp(roughness, 0.001, 0.999), 4.0) - 2.0;
    
    // Defining a vector with a direction from the pixel's world position, towards the light position.
    vec3 world_to_light_dir;
    
    // Initializing the color with the addition of the ambient component, and the emissive
    // component. Those components are indpendent from any light sources, from which we later calculate
    // the diffuse and the specular components.
    vec3 color = emissive + ambient_light * ambient;

    // Looping over all provided light sources, with a maximum of 8 light sources.
    for(int light_idx = 0; light_idx < min(MAX_LIGHTS, light_count); light_idx++){

        // Obtaining the current light source.
        Light light = lights[light_idx];

        // Initializing the attenuation = 1.0 . The attenuation of light governs the intensity of it, 
        // the more attenuation, the less intensity.
        float attenuation = 1.0;
        
        
        if(light.type == DIRECTIONAL){

            // If the light source is a directional light source, that is, if it has
            // only a direction and no position, then the direction should already be
            // passed from the cpp as a member of the light struct.
            world_to_light_dir = -light.direction;
        
        } else {

            // Otherwise: in cases of spot light sources and point light sources,
            // The direction is derived from the (world) position of the light and the
            // world position of the pixel we're rendering. We obtain it by subtrating
            // the two. We obtain the vector pointing from the pixel to the light source. 
            world_to_light_dir = light.position - fs_in.world;

            // We get the length of the vector to normalize it.
            // But also, we use it afterwards for the calculation of the attenuation.
            float d = length(world_to_light_dir);
            world_to_light_dir /= d;
            
            // Generally, the further a point is from a light source, the less light it will get.
            // By default, the attenuation should be = (1/d^2). That is, it's inversely proportionate to the
            // distance squared between the two.
            // But, for our purposes, we can choose that the attenuation be = (1/d) or even (1 => no attenuation 
            // whatever the position of the point). We pass a vector from the cpp for this purpose => light.attenuation.
            // For example:
            // light.attenuation * (d^2, d, 1.0) = (1.0, 0.0, 0.0) .  (d^2, d, 1.0) = d^2
            // light.attenuation * (d^2, d, 1.0) = (0.0, 1.0, 0.0) .  (d^2, d, 1.0) = d
            // Thus, we can gover how intense is the attenuation.s
            attenuation = 1.0 / dot(light.attenuation, vec3(d*d, d, 1.0));

            // In case this is a spot light, we need to compute an additional attenuation component.
            // It depends on the angle between the light direction, and the vector from the spot position
            // to the pixel position. (In spot light, the position of the light and the direction are separate).
            // The angle with respects to other angles (the inner cone angle, and the outer cone angle) determines
            // the intensity of the light.
            // Check the lab's slides for an illustration of the spot light. 
            if(light.type == SPOT){
                float angle = acos(dot(light.direction, -world_to_light_dir));
                attenuation *= smoothstep(light.cone_angles.y, light.cone_angles.x, angle);
            }
        }

        // The final computed diffuse is the multiplication of 
        vec3 computed_diffuse = light.color * diffuse * lambert(normal, world_to_light_dir);

        // We get the reflected light ray direction.
        // That is, the incident light ray but reflected around the normal of the surface.
        vec3 reflected = reflect(-world_to_light_dir, normal);

        // We compute the specular using phong model, multiplying 
        vec3 computed_specular = light.color * specular * phong(reflected, view, shininess);

        // We add to the color the final diffuse color, and the final specular color, 
        // both affected by the attenuation. (Something like ambient light, added to color above
        // is not affected by attenuation). 
        color += (computed_diffuse + computed_specular) * attenuation;

    }
    
    // Output the final computed color as the fragment color.
    frag_color = vec4(color, 1.0);
}