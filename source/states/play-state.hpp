#pragma once

#include "json/json.hpp"
#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>

#include "../common/components/free-camera-controller.hpp"
#include "../common/components/mesh-renderer.hpp"
#include "../common/components/camera.hpp"
#include "components/light.hpp"
#include "ecs/entity.hpp"
#include "our-util.hpp"
#include "playstate-type.hpp"
#include "systems/collision-detection.hpp"

#include "../common/text-utils.hpp"

#include <time.h>

// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::CollisionDetectionSystem collisionSystem;
    our::Text* currentPlayerText, *timeText, *numberOfCollectedArtifactsText;

    // This will indicate the total number of collectable artifacts. 
    int totalNumberOfArtifacts;

    // This records the time the player took to finish the race.
    float elapsedTime;

    void onInitialize() override {

        std::cout << "Play state type: " << type << std::endl;
        
        // Resetting time to be zero; 
        elapsedTime = 0;
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);

            our::CameraComponent* camera = nullptr;
            our::FreeCameraControllerComponent *controller = nullptr;
            // our::MeshRendererComponent *adjacent = nullptr;
            our::LightComponent *light = nullptr;
            for(auto entity : world.getEntities()){
                
                if (!camera || !controller) {
                    camera = entity->getComponent<our::CameraComponent>();
                    controller = entity->getComponent<our::FreeCameraControllerComponent>();
                }

                // If the entity holds a light component, add it to the set of light components.
                // This set of light components is later sent to the forward renderer after the
                // initialization with the function ForwardRenderer::setLights(std::unordered_set<LightComponent *> setOfLights)
                // For justification, see the relevant code parts in forward-renderer 
                light = entity->getComponent<our::LightComponent>();
                if (light) {
                    world.setOfLights.insert(light);
                }
                
            }
            /*
            for(auto entity : world.getEntities()){

                adjacent = entity->getComponent<our::MeshRendererComponent>();                
                if (adjacent) {
                    if (adjacent->isCameraAdjacent)
                        break;
                    else
                        adjacent = nullptr;
                }
                    
            }

            if (!adjacent || !camera || !controller) {
                std::cerr << "that is the problem" << std::endl;
                std::cerr << adjacent << std::endl;
                std::cerr << camera << std::endl;
                std::cerr << controller << std::endl;
            }

            adjacent->getOwner()->master = camera->getOwner();
            */

            // 
        }
        
        // We create the randomized artifacts.
        this->createRandomizedArtifacts();

        // We create the necessary text to be displayed, mainly: text to display time,
        // text to display the current player, and text to display the number of collected
        // artifacts out of total number of artifacts.
        currentPlayerText = our::CreateText(
            (this->type == 0) ? "USSR" : "US",
            1.5f,
            our::LEFT,
            glm::vec3(1.0f)
        );

        timeText = our::CreateText(
            // The displayed text will be replaced continuously in the onDraw() function below to 
            // indicate how much time elapsed.
            "PLACEHOLDER",
            1.5f,
            our::CENTER,
            glm::vec3(1.0f)
        );

        numberOfCollectedArtifactsText = our::CreateText(
            // The displayed text will be replaced continuously in the onDraw() function below to 
            // indicate how many artifacts have been collected so far.
            "PLACEHOLDER",
            1.5f,
            our::RIGHT,
            glm::vec3(1.0f)
        );

        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    // This function creates a random number of collectable artifacts at random locations
    // along the race track, between a minimum and a maximum number of total artifacts.
    void createRandomizedArtifacts() {

        srand(time(0));
        srandom(time(0));
        
        int minArtifacts = 40, maxArtifacts = 100;
        totalNumberOfArtifacts = rand() % (maxArtifacts - minArtifacts + 1);
        totalNumberOfArtifacts = totalNumberOfArtifacts + minArtifacts;

        std::cout << "The random number of artifacts will be: " << totalNumberOfArtifacts << std::endl;


        for (int i = 0; i < totalNumberOfArtifacts; i++) {

            our::Entity* newEntity = world.add();
            
            float zCoordinate = random() % 100;
            newEntity->localTransform.position = glm::vec3(0, 0, -zCoordinate);

            std::cout << glm::to_string(newEntity->localTransform.position) << std::endl;
            
            our::MeshRendererComponent *meshComponent = newEntity->addComponent<our::MeshRendererComponent>();
            meshComponent->mesh = our::AssetLoader<our::Mesh>::get("cube");
            meshComponent->material = our::AssetLoader<our::Material>::get("moon"); 

            world.setOfSpaceArtifacts.insert(newEntity);
        }
    }

    void onDraw(double deltaTime) override {
        
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        our::CameraComponent* camera = cameraController.update(&world, (float)deltaTime);
        int remainingCollectables = collisionSystem.update(&world, camera, getApp()->getSoundEngine());
        
        // And finally we use the renderer system to draw the scene
        renderer.render(&world);

        // Rendering currentPlayerText.
        glm::ivec2 windowSize = getApp()->getWindowSize();
        our::RenderText(
            currentPlayerText, 
            windowSize,
            getApp()->getCharacterMap()
        );

        // Updating time and rendering the time text.
        elapsedTime += deltaTime;
        timeText->displayedText = our::string_format("%02d:%02d", (int(elapsedTime)/60), (int)elapsedTime%60);
        our::RenderText(
            timeText,
            windowSize,
            getApp()->getCharacterMap()
        );

        // Rendering the text of the number of collected number of artifacts.
        numberOfCollectedArtifactsText->displayedText = our::string_format("%d/%d", totalNumberOfArtifacts - remainingCollectables, totalNumberOfArtifacts); 
        our::RenderText(
            numberOfCollectedArtifactsText,
            windowSize,
            getApp()->getCharacterMap()
        );

        // If you've finished your turn (here the check is only on the number of left collectables), take turns. 
        if (world.setOfSpaceArtifacts.empty()) {
            getApp()->setPlayerStats((our::PlaystateType)type, elapsedTime, 1.0);
            getApp()->takeTurns();
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }
};