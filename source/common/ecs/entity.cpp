#include "entity.hpp"
#include "../deserialize-utils.hpp"
#include "../components/component-deserializer.hpp"

#include <glm/gtx/euler_angles.hpp>

namespace our {

    // This function returns the transformation matrix from the entity's local space to the world space
    // Remember that you can get the transformation matrix from this entity to its parent from "localTransform"
    // To get the local to world matrix, you need to combine this entities matrix with its parent's matrix and
    // its parent's parent's matrix and so on till you reach the root.
    glm::mat4 Entity::getLocalToWorldMatrix() const {
        
        //DONE: (Req 8) Write this function

        // Comment:
        // This is simple iteration over the ancestors of the current entity.
        // We multiply the chain of the localToWorld (M) matrices, and return the product.
        
        glm::mat4 localToWorldMatrix(this->localTransform.toMat4());
        Entity* it = this->parent;
        
        while (it) {
            localToWorldMatrix = it->localTransform.toMat4() * localToWorldMatrix;
            it = it->parent;
        }

        return localToWorldMatrix;
    }

    // Deserializes the entity data and components from a json object
    void Entity::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        name = data.value("name", name);

        // Obtaining the type of the mesh (if any) included in the entity
        std::string t = data.value("typeOfGenerated", "OTHER");
        if (t == "CELESTIAL_ORB") typeOfChildMesh = CELESTIAL_ORB;
        else if (t == "COLLECTABLE_COIN") typeOfChildMesh = COLLECTABLE_COIN;
        else if (t == "MAIN_AIRCRAFT") typeOfChildMesh = MAIN_AIRCRAFT; 
        else if (t == "TRACK") typeOfChildMesh = TRACK;
        else if (t == "FINISH_LINE") typeOfChildMesh = FINISH_LINE;
        else typeOfChildMesh = OTHER;

        localTransform.deserialize(data);
        if(data.contains("components")){
            if(const auto& components = data["components"]; components.is_array()){
                for(auto& component: components){
                    deserializeComponent(component, this);
                }
            }
        }
    }

}