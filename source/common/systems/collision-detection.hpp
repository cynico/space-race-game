#pragma once

#include "../components/camera.hpp"
#include "../ecs/world.hpp"

#include <glm/glm.hpp>
#include <unordered_set>

#include <irrKlang.h>
#include <conio.h>


namespace our {
class CollisionDetectionSystem {
public:
    // This should be called every frame to check if any collisions happened.
    int update(World *world, our::CameraComponent *camera, irrklang::ISoundEngine* engine) {

        if (!world || !camera)
            return 0;

        std::unordered_set<Entity*> todelete;
        
        // Looping over all space artifacts, and checking if collision happened with any of them.
        // TODO: this may be enhanced by breaking from the loop once a collision happened, guaranteed no two artifacts
        // will ever be in the same position.
        for (auto it = world->setOfSpaceArtifacts.begin(); it != world->setOfSpaceArtifacts.end(); it++) {
            float distance = glm::distance((*it)->localTransform.position, camera->getOwner()->localTransform.position);
            if (distance < 3.0) {
                world->markForRemoval(*it);
                todelete.insert(*it);
                engine->play2D("assets/sounds/coin.wav");
            }
        }

        // Removing the deleted artifacts from the set of space artifacts.
        // This should be separate to the above loop so that everything is not fucked up.
        for (auto it = todelete.begin(); it != todelete.end(); it++) {
            world->setOfSpaceArtifacts.erase(*it);
        }
        todelete.clear();

        // Actually deleting the marked entities. This helps in saving unused memory.
        world->deleteMarkedEntities();

        // Returning the remaining number of the collectable space artifacts.
        return world->setOfSpaceArtifacts.size();
  }
};

} // namespace our
