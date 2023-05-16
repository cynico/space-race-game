#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"
#include "glad/gl.h"
#include "shader/shader.hpp"

#include <iostream>

namespace our {

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const {
        
        //DONE: (Req 7) Write this function
        pipelineState.setup();
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
        TintedMaterial::setup();
        
        // Setting the alphaThreshold uniform used to discard fragments.
        shader->use();
        shader->set("alphaThreshold", alphaThreshold);

        // Binding the texture and the sampler to texture unit 0.
        glActiveTexture(GL_TEXTURE0);
        if (texture != NULL) {
            texture->bind();
        }
        
        if (sampler != NULL) {
            sampler->bind(0);
        }

        // Sending the value 0 as the value of the uniform tex.
        shader->set("tex", 0);
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    void LitMaterial::setup() const {

        TintedMaterial::setup();

        // Setting the alphaThreshold uniform used to discard fragments.
        shader->use();
        shader->set("alphaThreshold", alphaThreshold);


        // Binding Textures: albedo, specular, roughness, ambient_occlusion, and emissive.
        glActiveTexture(GL_TEXTURE0);
        albedo->bind();
        sampler->bind(0);
        shader->set("material.albedo", 0);

        glActiveTexture(GL_TEXTURE1);
        specular->bind();
        sampler->bind(1);
        shader->set("material.specular", 1);

        glActiveTexture(GL_TEXTURE2);
        roughness->bind();
        sampler->bind(2);
        shader->set("material.roughness", 2);

        glActiveTexture(GL_TEXTURE3);
        ambient_occlusion->bind();
        sampler->bind(3);
        shader->set("material.ambient_occlusion", 3);

        glActiveTexture(GL_TEXTURE4);
        emissive->bind();
        sampler->bind(4);
        shader->set("material.emissive", 4);


    }

    void LitMaterial::deserialize(const nlohmann::json &data) {
        TintedMaterial::deserialize(data);
        if (!data.is_object()) return;

        alphaThreshold = data.value("alphaThreshold", 0.0f);
        albedo = AssetLoader<Texture2D>::get(data.value("albedo", ""));
        specular = AssetLoader<Texture2D>::get(data.value("specular", ""));
        roughness = AssetLoader<Texture2D>::get(data.value("roughness", ""));
        ambient_occlusion = AssetLoader<Texture2D>::get(data.value("ambient_occlusion", ""));
        emissive = AssetLoader<Texture2D>::get(data.value("emissive", ""));

        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));

    }

    void TexturedGIFMaterial::deserialize(const nlohmann::json &data) {
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        
        gif = AssetLoader<GIFTexture>::get(data.value("gif", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    void TexturedGIFMaterial::setup() const {

        TintedMaterial::setup();

        shader->use();

        shader->set("alphaThreshold", alphaThreshold);

        // Binding the texture and the sampler to texture unit 0.
        glActiveTexture(GL_TEXTURE0);
        if (gif->textures[currentFrame] != NULL) {
            gif->textures[currentFrame]->bind();
        }

        if (sampler != NULL) {
            sampler->bind(0);
        }

        // Sending the value 0 as the value of the uniform tex.
        shader->set("tex", 0);
    }
}