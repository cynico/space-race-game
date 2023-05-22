#include "world.hpp"
#include "components/mesh-renderer.hpp"
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

            // If this is the aircraft entity, set the world->airCraftEntity to point to it.
            if (newEntity->typeOfChildMesh == our::MAIN_AIRCRAFT) this->airCraftEntity = newEntity;
        }
    }

    // This function should set all the track-related variables in the our::Track track
    // data member above. Those include: the far left, far right, nearest z, and furthest z. 
    // Remember: the track is a MultipleMeshesRendererComponent. That is, it contains multiple
    // meshes, each with their farLeft, farRight, zFurthest, and zNearest. We just compare them
    // and end up with the furthest point on the left in the whole track, the right, etc.
    void World::setTrackRelatedVariables(our::MultipleMeshesRendererComponent* track) {
        
        glm::vec4 farLeft, farRight, zFurthest, zNearest;
        for (auto it = track->meshes->listOfMeshes->begin(); it != track->meshes->listOfMeshes->end(); it++) {
            
            farLeft = track->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->farLeft, 1);
            farRight = track->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->farRight, 1);
            zFurthest = track->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->zFurthest, 1);
            zNearest = track->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->zNearest, 1);

            if (farLeft.x < this->track.tracksFarLeft.x)
                this->track.tracksFarLeft = glm::vec3(farLeft);

            if (farRight.x > this->track.tracksFarRight.x)
                this->track.tracksFarRight = glm::vec3(farRight);

            if (zFurthest.z < this->track.tracksZFurthest)
                this->track.tracksZFurthest = zFurthest.z;
            
            if (zNearest.z > this->track.tracksZNearest)
                this->track.tracksZFurthest = zFurthest.z;
        }
    }

}