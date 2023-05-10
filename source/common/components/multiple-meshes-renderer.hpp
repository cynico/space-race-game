#pragma once

#include "../ecs/component.hpp"
#include "../mesh/mesh.hpp"
#include "../material/material.hpp"
#include "../asset-loader.hpp"
#include "mesh/multiple-meshes.hpp"

namespace our {

    // This MultipleMeshesRendererComponent is an equivalent to the MeshRendererComponent
    // Only this one contains a list of meshes instead of one mesh, and a list of materials
    // instead of one material. 
    class MultipleMeshesRendererComponent : public Component {
    public:
        MultipleMeshes* meshes; // The meshes that should be drawn
        std::list<Material*> *materials; // The materials used to draw the meshes

        // The ID of this component type is "Multiple Mesh Renderer"
        static std::string getID() { return "Multiple Mesh Renderer"; }

        // Receives the meshes & materiales from the AssetLoader by the names given in the json object
        void deserialize(const nlohmann::json& data) override;
    };

}