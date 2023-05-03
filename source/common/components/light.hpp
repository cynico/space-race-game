#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

namespace our {

    enum LightType {
        DIRECTIONAL,
        POINT,
        SPOT
    };


    class LightComponent : public Component {
    public:
        LightType type;
        glm::vec3 color = glm::vec3(0.8, 0.8, 0.8); // Using default color of white-ish gray.
        glm::vec2 cone_angles = glm::vec2(0.0, 0.0);
        glm::vec3 attenuation = glm::vec3(1.0, 0.0, 0.0); // Using default attenuation of 1/(d^2)
        glm::vec3 direction = glm::vec3(1.0, 1.0, 0.0); // Direction of light.

        // Reads the light's data from the given json object.
        void deserialize(const nlohmann::json& data) override;

        // The ID of this component type is "Light"
        static std::string getID() { return "Light"; }

        // Prints all related info of LightComponent, for debugging purposes.
        void PrintYourLightInfo();

    };
};