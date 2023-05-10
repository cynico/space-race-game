#pragma once

#include "./mesh.hpp"
#include <array>
#include <list>
#include <glm/glm.hpp>
#include <material/material.hpp>

namespace our {


    class MultipleMeshes {
    public:
        std::list<Mesh*>* listOfMeshes;
        MultipleMeshes(std::list<Mesh*>* meshes) {
            this->listOfMeshes = meshes;
        }
    };

};