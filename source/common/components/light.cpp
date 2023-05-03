#include "light.hpp"
#include <iostream>
#include "../deserialize-utils.hpp"

namespace our {

    void LightComponent::PrintYourLightInfo()  {

        std::cout << "Printing my light info" << std::endl;
        std::cout << "type: " << type << std::endl;
        std::cout << "color: " << glm::to_string(color) << std::endl;
        std::cout << "cone_angles: " << glm::to_string(cone_angles) << std::endl;
        std::cout << "attenuation: " << glm::to_string(attenuation) << std::endl << std::endl;

    }

    void LightComponent::deserialize(const nlohmann::json &data) {
        
        if (!data.is_object()) return;

        std::string lightType = data.value("lightType", "DIRECTIONAL");
        if (lightType == "DIRECTIONAL") {
            type = DIRECTIONAL;
            direction = data.value("direction", direction);
        } else if (lightType == "POINT") {
            type = POINT;
            attenuation = data.value("attenuation", attenuation);
        } else if (lightType == "SPOT") {
            type = SPOT;
            cone_angles = data.value("cone_angles", cone_angles);
            attenuation = data.value("attenuation", attenuation);
            direction = data.value("direction", direction);
        } else {
            std::cerr << "ERROR: unkown light type: " << lightType << std::endl;
            return;
        }
        
        color = data.value("color", color);
    }
    
};

