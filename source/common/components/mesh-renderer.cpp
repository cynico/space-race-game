#include "mesh-renderer.hpp"
#include "../asset-loader.hpp"
#include "material/material.hpp"
#include <iostream>
#include "../ecs/entity.hpp"

namespace our {
    // Receives the mesh & material from the AssetLoader by the names given in the json object
    void MeshRendererComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        // Notice how we just get a string from the json file and pass it to the AssetLoader to get us the actual asset
        //DONE: (Req 8) Get the material and the mesh from the AssetLoader by their names
        // which are defined with the keys "mesh" and "material" in data.
        // Hint: To get a value of type T from a json object "data" where the key corresponding to the value is "key",
        // you can use write: data["key"].get<T>().
        // Look at "source/common/asset-loader.hpp" to know how to use the static class AssetLoader.

        // Comment:
        // This is self-explanatory given the above comment.
        // I've used moon for the default material and monkey for the default mesh, because why not.
        std::string material = data.value("material", "moon");
        std::string mesh = data.value("mesh", "monkey");
        this->material = AssetLoader<Material>::get(material);
        this->mesh = AssetLoader<Mesh>::get(mesh);
    }
}