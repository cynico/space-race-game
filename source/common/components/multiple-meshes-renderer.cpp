#include "multiple-meshes-renderer.hpp"
#include "../asset-loader.hpp"
#include "material/material.hpp"
#include <iostream>
#include <stdexcept>
#include "../ecs/entity.hpp"
#include "mesh/multiple-meshes.hpp"
#include "shader/shader.hpp"

namespace our {
    
    void MultipleMeshesRendererComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

        // Obtaining the name of the multiple-mesh component, with a default value of "track".
        std::string mesh = data.value("multiple-mesh", "track");

        // Obtaining the list of meshes from the asset loader, encapsulated by a MultipleMeshes class. 
        this->meshes = AssetLoader<MultipleMeshes>::get(mesh);

        // Obtaining a list of corresponding size of materials for the above meshes. 
        // First: creating the list.
        this->materials = new std::list<Material *>();

        // Checking if the JSON contains "materials" key.
        if(data.contains("materials")){
            // Obtaining the object that has the key "materials". Checking if it is an array.
            if(const auto& materials = data["materials"]; materials.is_array()){
                // Looping over all materials, getting them from the asset loader, and pushing them back to the list
                for(std::string material: materials){
                    this->materials->push_back(AssetLoader<Material>::get(material));
                }
            } else {
                throw std::runtime_error("UNEXPECTED:: PROVIDED MATERIALS FOR MULTIPLE MESH RENDERER IS NOT AN ARRAY.");
            }
        } else {
            // If not, throw a runtime error.
            throw std::runtime_error("UNEXPECTED:: NO PROVIDED MATERIALS FOR MULTIPLE MESH RENDERER.");
        }

        // If the provided list of materials is not equal in size to the number of objects in the mesh
        // .obj file, throw an exception.
        if (this->meshes->listOfMeshes->size() != this->materials->size())
            throw std::runtime_error("UNEXPECTED:: NUMBER OF OBJECTS IN PROVIDED MESH IS NOT EQUAL TO NUMBER OF PROVIDED MATERIALS");

        // Setting the name of the mesh's object as listed in config.json 
        nameOfMeshObject = mesh;
    }
    
}