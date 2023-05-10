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
#include "../common/components/movement.hpp"

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

        this->createRandomizedEnvironment();

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
            newEntity->typeOfChildMesh = our::COLLECTABLE_COIN;

            float zCoordinate = random() % 100;
            newEntity->localTransform.position = glm::vec3(0, 0, -zCoordinate);
            
            our::MeshRendererComponent *meshComponent = newEntity->addComponent<our::MeshRendererComponent>();
            meshComponent->mesh = our::AssetLoader<our::Mesh>::get("cube");
            meshComponent->material = our::AssetLoader<our::Material>::get("moon"); 

            assert((newEntity->getComponentsSize() == 1));

            world.setOfSpaceArtifacts.insert(newEntity);
        }
    }

    // This function creates a random environment of planets, moons, and suns.
    void createRandomizedEnvironment() {

        std::unordered_set<our::Entity *> setOfCelestialOrbs;

        // This will be used for randomizing the sign of the random x and y 
        // components generated below.
        int sign[2] = {1, -1};
        std::string planets[2] = {"planet-1", "planet-2"};
        
        // Setting the seed of the integer random number generator.
        srand(time(0));
        srandom(time(0));

        // Generating a random number of planets, between minPlanets
        // and maxPlanets.
        int minPlanets = 50, maxPlanets = 100, minY = 20, maxY = 120;
        float scale, xCoordinate, yCoordinate, zCoordinate, minimumDistance = 70.0;

        int totalNumberOfPlanets = rand() % (maxPlanets - minPlanets + 1);
        totalNumberOfPlanets += minPlanets;
        std::cout << "The random number of planets will be: " << totalNumberOfPlanets << std::endl;

        // Creating totalNumberOfPlanets planets.
        for (int i = 0; i < totalNumberOfPlanets; i++) {
            
            // Creating a new entity and adding it to the world.
            our::Entity* newPlanet = world.add();
            newPlanet->typeOfChildMesh = our::CELESTIAL_ORB;
            
            // Generating random scale between 2 and 5
            scale = (random() % (5 - 2 + 1)) + 2;
            
            // Randomizing coordinates, and looping until we make sure that the new planet
            // is of minimum required distance to all other planets.
            bool invalid = false;
            do {
                xCoordinate = (sign[random() % 2]) * (random() % 240);
                yCoordinate = (sign[random() % 2]) * ((random() % (maxY - minY + 1)+minY));
                zCoordinate = - random() % 500;
                newPlanet->localTransform.position = glm::vec3(xCoordinate, yCoordinate, zCoordinate);
                for (auto it = setOfCelestialOrbs.begin(); it != setOfCelestialOrbs.end(); it++) {
                    if (glm::distance(newPlanet->localTransform.position, (*it)->localTransform.position) < minimumDistance) {
                        invalid = true;
                        break;
                    } else
                        invalid = false;
                }

            } while (invalid);

            // Randomizing scale.
            newPlanet->localTransform.scale = glm::vec3(scale);
            
            // Creating the MeshRenderer component, which will be a sphere,
            // and will have a material with a planet texture, that will be randomized
            // from the ones in the planets array above.
            our::MeshRendererComponent *meshComponent = newPlanet->addComponent<our::MeshRendererComponent>();
            meshComponent->mesh = our::AssetLoader<our::Mesh>::get("sphere");
            meshComponent->material = our::AssetLoader<our::Material>::get(planets[random() % 2]);

            // Adding a movement component to the planet, that is equivalent of the planet's rotation
            // around its own axis. 
            our::MovementComponent* rotationComponent = newPlanet->addComponent<our::MovementComponent>();
            rotationComponent->angularVelocity = glm::vec3(0.0, ((random() % 120) / 180.0) * glm::pi<float>(), 0);

            setOfCelestialOrbs.insert(newPlanet);

            // Creating a moon around the planet.
            // We create a moon around every 1 out of 3 planets.
            if (random() % 3 > 1) {
                
                // Creating the moon entity.
                // Setting the scale of the sphere to be 1/4th of the scale of the planet around which it rotates.
                // Setting the position of the moon close to the planet (2.5*scale). This value was hand-picked,
                // according to what seemed tastable. 
                our::Entity* moon = world.add();
                moon->typeOfChildMesh = our::CELESTIAL_ORB;
                moon->localTransform.scale = glm::vec3(scale/4);
                moon->localTransform.position = glm::vec3(newPlanet->localTransform.position) + glm::vec3(scale*2.5);

                // Creating the moon mesh, and setting its material.
                our::MeshRendererComponent *moonMesh = moon->addComponent<our::MeshRendererComponent>();
                moonMesh->mesh = our::AssetLoader<our::Mesh>::get("sphere");
                moonMesh->material = our::AssetLoader<our::Material>::get("moon");

                // Inserting the moon into the setOfCelestialOrbs, to calculate the distance later with
                // other planets/suns/moons.
                setOfCelestialOrbs.insert(moon);
            }
        }


        // Creating stars.
        int totalNumberOfStars = rand() % (10 - 5 + 1);
        totalNumberOfStars += 5;
        std::cout << "The random number of stars will be: " << totalNumberOfStars << std::endl;

        for (int i = 0; i < totalNumberOfStars; i++) {
                 
            // Creating a new entity and adding it to the world.
            our::Entity* newStar = world.add();
            newStar->typeOfChildMesh = our::CELESTIAL_ORB;
            

            // Randomizing coordinates, and looping until we make sure that the new planet
            // is of minimum required distance to all other planets.
            bool invalid = false;
            do {
                xCoordinate = (sign[random() % 2]) * (random() % 240);
                yCoordinate = (sign[random() % 2]) * ((random() % (maxY - minY + 1)+minY));
                zCoordinate = - random() % 500;
                newStar->localTransform.position = glm::vec3(xCoordinate, yCoordinate, zCoordinate);
                for (auto it = setOfCelestialOrbs.begin(); it != setOfCelestialOrbs.end(); it++) {
                    if (glm::distance(newStar->localTransform.position, (*it)->localTransform.position) < minimumDistance) {
                        invalid = true;
                        break;
                    } else
                        invalid = false;
                }

            } while (invalid);

            // Randomizing scale between 2 and 5
            scale = (random() % (7 - 4 + 1)) + 4;
            newStar->localTransform.scale = glm::vec3(scale);
            
            // Creating the MeshRenderer component, which will be a sphere,
            // and will have a material with a planet texture, that will be randomized
            // from the ones in the planets array above.
            our::MeshRendererComponent *starMesh = newStar->addComponent<our::MeshRendererComponent>();
            starMesh->mesh = our::AssetLoader<our::Mesh>::get("sphere");
            starMesh->material = our::AssetLoader<our::Material>::get("star");

            // Creating a (point) light component and adding it to the star.
            our::LightComponent *starLight = newStar->addComponent<our::LightComponent>();
            starLight->type = our::POINT; starLight->attenuation = glm::vec3(0,  1, 0); // attenuation 1/d for now
            starLight->color = glm::vec3(1.0);

            world.setOfLights.insert(starLight);

            // Adding a movement component to the star, that is equivalent of the star's rotation
            // around its own axis.
            our::MovementComponent* starRotation = newStar->addComponent<our::MovementComponent>();
            starRotation->angularVelocity = glm::vec3(0.0, ((random() % 60) / 180.0) * glm::pi<float>(), 0);

            setOfCelestialOrbs.insert(newStar);

        }

        setOfCelestialOrbs.clear();
    }

    void onDraw(double deltaTime) override {
        
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        
        // forbiddenAccess: whether the player has tried to enter a forbidden zone.
        // forbiddenCollision: whether the player has collided with a planet. 
        bool forbiddenAccess = false, forbiddenCollision = false;
        
        // Get the camera component, and fetch a boolean whether the player
        // has tried to access a forbidden zone. (Anything behind a certain point on the z-axis)
        // This boolean is then passed to the renderer in the render function.

        // Here, we get the new position of the camera in the updatedCameraPosition vector.
        // We don't update the position of the camera directly, we take updatedCameraPosition, 
        // check if there is any forbidden collisions first, and if there is not, we update
        // the actual camera position to be updatedCameraPosition.
        glm::vec3 updatedCameraPosition;
        our::CameraComponent* camera = cameraController.update(&world, (float)deltaTime, &updatedCameraPosition, &forbiddenAccess);
        
        // Obtain the remaining number of collectables to update the text.
        int remainingCollectables = collisionSystem.update(&world, updatedCameraPosition, getApp()->getSoundEngine(), &forbiddenCollision, (our::PlaystateType)this->type);

        // If a forbidden collision has not happen, update the actual camera position.
        if (!forbiddenCollision) {
            // actually updating the position of the camera.
            camera->setPosition(updatedCameraPosition);
        }
        // And finally we use the renderer system to draw the scene
        renderer.render(&world, forbiddenAccess);

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