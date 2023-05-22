#pragma once

#include "../components/camera.hpp"
#include "../ecs/world.hpp"
#include "ecs/entity.hpp"

#include <glm/glm.hpp>
#include <unordered_set>

#include <irrKlang.h>
#include <conio.h>
#include "../states/extra-definitions.hpp"


namespace our {
class CollisionDetectionSystem {
public:
    
    // This should be called every frame to check if any collisions happened.
    // Two types of collisions are detected:
    // - Collisions with coins (allowed because they are collectable, the coins should disappear)
    // - Collisions with celestial orbs/planets (not allowed)
    // The argument forbiddenCollision is a reference to a boolean, that is set to true if a forbidden
    // collision (one with a planet) is detected.

    // There is an assumption made in this function, if violated the funciton will need to be changed:
    // a collectable and a planet will never be rendered closely in the world. This is now guaranteed because
    // we render the coins on the track, and the planets everywhere else.
    int update(World *world, glm::vec3 updatedPosition, irrklang::ISoundEngine* engine, bool *forbiddenCollision, our::SpeedCollectableInfo *speed) {

        if (!world)
            return 0;

        std::unordered_set<Entity*> collected;
        std::unordered_set<Entity*> toCollectIfSpeed;

        // Initialize it to be false.
        *forbiddenCollision = false;

        // Iterate over all the entities in the world.
        for (auto it = world->getEntities().begin(); it != world->getEntities().end(); it++) {
            
            // In case the entity is a collectable coin.                         
            if ((*it)->typeOfChildMesh == our::COLLECTABLE_COIN) {
                
                // Caclulate distance between the new to-be camera position, and the coin.  
                float distance = glm::distance((*it)->localTransform.position, updatedPosition);
                // If the distance is less than a manually fine-tuned threshold, then remove the coin
                // from the world, and play a sound.
                if (distance < 3.5) {                    
                    collected.insert(*it);
                    engine->play2D("assets/sounds/coin.wav");
                    continue;
                }

                // If speedup is in effect, and we pass by the current coin (the coin is situated after 
                // the point where we gained the speedup effect), collect it.
                // Note: speed->zAtTimeOfCollection == 100.0 when speedup is not in effect.
                // This may seem equivalent to using speed->inEffect as a boolean, but it is not.
                if ( 
                    (speed->zAtTimeOfCollection != 100.0) && 
                    ((*it)->localTransform.position.z < speed->zAtTimeOfCollection) &&
                    abs((*it)->localTransform.position.z - updatedPosition.z) <= 1
                ) toCollectIfSpeed.insert(*it);
            
            
            }             
            // If this is a speed collectable, set speed->inEffect=true, and remove the collectable.
            else if ((*it)->typeOfChildMesh == our::SPEED_COLLECTABLE) {
                
                float distance = glm::distance((*it)->localTransform.position, updatedPosition);
                if (distance < 3.0) {
                    speed->inEffect = true;
                    collected.insert(*it);
                } else {
                    speed->inEffect = false;
                }

            }    

            // In case the entity is a celestial orb (planet).
            else if ((*it)->typeOfChildMesh == our::CELESTIAL_ORB) {
                
                // Calculate the distance, and check whether it's smaller than a threshold.
                // Now, this threshold was chosen manually. For a sphere of scale (1,1,1),
                // a distance of 2.2 seemed appropriate and not too invading.
                // We multiply this 2.2 by the scale (any component in it, x = y = z, we alwas perform uniform
                // scaling with the planets), to get a threshold appropriate to the size of the planet.  

                // If the distance is smaller than the threshold, set forbiddenCollision=true, and break.
                float distance = glm::distance((*it)->localTransform.position, updatedPosition);
                if (distance < 2.2 * (*it)->localTransform.scale.x ) {
                    *forbiddenCollision = true;
                    break;
                }
            } else if ((*it)->typeOfChildMesh == our::OTHER_AIRCRAFTS) {

                // Calculate the distance, and check whether it's smaller than a threshold.
                // Now, this threshold was chosen manually. For an aircraft of scale (1,1,1),
                // a distance of 2.2 seemed appropriate and not too invading.
                // We multiply this 2.2 by the scale (any component in it, x = y = z, we alwas perform uniform
                // scaling with the planets), to get a threshold appropriate to the size of the aircraft.  
                float distance = glm::distance((*it)->localTransform.position, updatedPosition);
                if (distance < 15 * (*it)->localTransform.scale.x * world->airCraftEntity->localTransform.scale.x) {
                    *forbiddenCollision = true;
                    break;
                }            
            }
        }

        // If speed mode is in effect, collect all the coins in approximity to you.
        if (speed->timeSince != 0.0) {
            for (auto it = toCollectIfSpeed.begin(); it != toCollectIfSpeed.end(); it++) {
                world->markForRemoval(*it);
                world->setOfSpaceArtifacts.erase(*it);
                engine->play2D("assets/sounds/coin.wav");
            }
        }

        toCollectIfSpeed.clear();

        // Removing the deleted artifacts from the set of space artifacts.
        // This should be separate to the above loop so that everything is not fucked up.
        for (auto it = collected.begin(); it != collected.end(); it++) {
            world->markForRemoval(*it);
            world->setOfSpaceArtifacts.erase(*it);
        }
        collected.clear();

        // Actually deleting the marked entities. This helps in saving unused memory.
        world->deleteMarkedEntities();

        // Returning the remaining number of the collectable space artifacts.
        return world->setOfSpaceArtifacts.size();
  }
};

}
