#include "world.hpp"
#include "components/multiple-meshes-renderer.hpp"
#include "ecs/entity.hpp"
#include <limits>

namespace our {

    // This will deserialize a json array of entities and add the new entities to the current world
    // If parent pointer is not null, the new entities will be have their parent set to that given pointer
    // If any of the entities has children, this function will be called recursively for these children
    void World::deserialize(const nlohmann::json& data, Entity* parent){

        if(!data.is_array()) return;
        for(const auto& entityData : data){
            //DONE: (Req 8) Create an entity, make its parent "parent" and call its deserialize with "entityData".
            Entity *newEntity = this->add();
            newEntity->parent = parent;
            newEntity->deserialize(entityData); // deserializing components

            if(entityData.contains("children")){
                //DONE: (Req 8) Recursively call this world's "deserialize" using the children data
                // and the current entity as the parent

                this->deserialize(entityData["children"], newEntity);
            }
        }

        // This part only executes at the top function call. Remember, this is a recursive function.
        if (!parent) {
            // Find the track MultipleMeshRednerComponent, compute the farRight and the farLeft points
            // and save them in this->tracksFarRight and this->tracksFarLeft.
            // This is used later for restricting motion along the x-axis.
            for (auto entity = this->getEntities().begin(); entity != this->getEntities().end(); entity++) {
                
                if (MultipleMeshesRendererComponent* c = (*entity)->getComponent<MultipleMeshesRendererComponent>(); c)
                    if (c->nameOfMeshObject == "track") {
                        
                        for (auto it = c->meshes->listOfMeshes->begin(); it != c->meshes->listOfMeshes->end(); it++) {
                            
                            glm::vec4 farLeftWorldCoordinates = (*entity)->getLocalToWorldMatrix() * glm::vec4((*it)->farLeft, 1);
                            glm::vec4 farRightWorldCoordinates = (*entity)->getLocalToWorldMatrix() * glm::vec4((*it)->farRight, 1);

                            if (farLeftWorldCoordinates.x < this->tracksFarLeft.x)
                                this->tracksFarLeft = glm::vec3(farLeftWorldCoordinates.x, farLeftWorldCoordinates.y, farLeftWorldCoordinates.z);

                            if (farRightWorldCoordinates.x > this->tracksFarRight.x)
                                this->tracksFarRight = glm::vec3(farRightWorldCoordinates.x, farRightWorldCoordinates.y, farRightWorldCoordinates.z);

                        }
                        break;
                   }
            }
        }
    }

}