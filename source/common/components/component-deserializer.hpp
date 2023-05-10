#pragma once

#include "../ecs/entity.hpp"
#include "camera.hpp"
#include "components/light.hpp"
#include "components/multiple-meshes-renderer.hpp"
#include "mesh-renderer.hpp"
#include "free-camera-controller.hpp"
#include "movement.hpp"
#include "light.hpp"

namespace our {

    // Given a json object, this function picks and creates a component in the given entity
    // based on the "type" specified in the json object which is later deserialized from the rest of the json object
    inline void deserializeComponent(const nlohmann::json& data, Entity* entity){
        std::string type = data.value("type", "");
        Component* component = nullptr;
        //DONE: (Req 8) Add an option to deserialize a "MeshRendererComponent" to the following if-else statement
        if(type == CameraComponent::getID()){
            component = entity->addComponent<CameraComponent>();
        } else if (type == FreeCameraControllerComponent::getID()) {
            component = entity->addComponent<FreeCameraControllerComponent>();
        } else if (type == MovementComponent::getID()) {
            component = entity->addComponent<MovementComponent>();
        } else if (type == MeshRendererComponent::getID()) {
            component = entity->addComponent<MeshRendererComponent>();
        } else if (type == LightComponent::getID()) {
            component = entity->addComponent<LightComponent>();
        } else if (type == MultipleMeshesRendererComponent::getID()) {
            component = entity->addComponent<MultipleMeshesRendererComponent>();
        }
        if(component) component->deserialize(data);
        // Printing the info of the component if it is a light source.
        /*if (type == LightComponent::getID())  {
            dynamic_cast<LightComponent*>(component)->PrintYourLightInfo();
        }*/
    }

}