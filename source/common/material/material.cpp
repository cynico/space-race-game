#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"
#include "glad/gl.h"
#include "shader/shader.hpp"

#include <iostream>

namespace our {

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const {
        
        //DONE_RETURN_HERE_AGAIN: (Req 7) Write this function
        pipelineState.setup();
        
        /*
            if (dynamic_cast<const TexturedMaterial*>(this)) {
                shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
                shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            } else if (dynamic_cast<const TintedMaterial*>(this)) {
                std::cout << "tinted" << std::endl;
                if (!shader->attach("assets/shaders/tinted.vert", GL_VERTEX_SHADER)) {
                    std::cerr << "error attaching vertex shader" << std::endl;
                } else {
                    std::cout << "correct" << std::endl;
                }
                if (!shader->attach("assets/shaders/tinted.frag", GL_FRAGMENT_SHADER)) {
                    std::cerr << "error attaching fragment shader" << std::endl;
                } else {
                    std::cout << "correct" << std::endl;
                }
                if (!shader->link()) {
                    std::cout << "why" << std::endl;
                }
            }
        */
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

        if(data.contains("pipelineState")){
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint 
    void TintedMaterial::setup() const {
        
        //DONE: (Req 7) Write this function
        Material::setup();
        shader->use();
        shader->set("tint", tint);

    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json& data){
        Material::deserialize(data);
        if(!data.is_object()) return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex" 
    void TexturedMaterial::setup() const {
        
        //DONE: (Req 7) Write this function

        Material::setup();
        
        // Using the shader program and setting the alphaThreshold uniform.
        shader->use();
        shader->set("alphaThreshold", alphaThreshold);

        // Binding the texture and the sampler to texture unit 0.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getOpenGLName());
        sampler->bind(0);
        
        // Sending the value 0 as the value of the uniform tex.
        shader->set("tex", 0);
        shader->set("tint", tint);


    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

}