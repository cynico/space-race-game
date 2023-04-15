#include "entity.hpp"
#include "../deserialize-utils.hpp"
#include "glm/ext/matrix_transform.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <iostream>

namespace our {

    void printMatrix(glm::mat4 matrix) {

        std::cout << "[" << std::endl;

        for (int i = 0; i <  4; i++ ) {
            std::cout << matrix[0][i] << ' ' << matrix[1][i] << ' ' <<  matrix[2][i] << ' ' << matrix[3][i];  
            std::cout << std::endl;
        }

        std::cout << "]" << std::endl;
    }

    // This function computes and returns a matrix that represents this transform
    // Remember that the order of transformations is: Scaling, Rotation then Translation
    // HINT: to convert euler angles to a rotation matrix, you can use glm::yawPitchRoll
    glm::mat4 Transform::toMat4() const {
        
        //DONE: (Req 3) Write this function
        // Order: Scaling, rotation, translation. (The order is reversed in the multiplication order)

        // Translation
        glm::mat4 transformationMat = glm::mat4(1.0f);
        transformationMat = glm::translate(transformationMat, position);

        // Rotation
        glm::mat4 rotationMat = glm::yawPitchRoll(rotation[1], rotation[0], rotation[2]);
        transformationMat = transformationMat * rotationMat;

        // Scaling
        transformationMat = glm::scale(transformationMat, scale);

        return transformationMat;
    }

     // Deserializes the entity data and components from a json object
    void Transform::deserialize(const nlohmann::json& data){
        position = data.value("position", position);
        rotation = glm::radians(data.value("rotation", glm::degrees(rotation)));
        scale    = data.value("scale", scale);
    }

}