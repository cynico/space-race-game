#pragma once

#include "json/json.hpp"
#include <X11/Xlib.h>
#include <application.hpp>

#include <cassert>
#include <cstdlib>
#include <ecs/world.hpp>
#include <limits>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>

#include "../common/components/free-camera-controller.hpp"
#include "../common/components/mesh-renderer.hpp"
#include "../common/components/camera.hpp"
#include "GLFW/glfw3.h"
#include "components/light.hpp"
#include "components/multiple-meshes-renderer.hpp"
#include "ecs/entity.hpp"
#include "material/material.hpp"
#include "our-util.hpp"
#include "extra-definitions.hpp"
#include "shader/shader.hpp"
#include "systems/collision-detection.hpp"

#include "../common/text-utils.hpp"
#include "../common/our-util.hpp"

#include "../common/components/movement.hpp"
#include "texture/texture2d.hpp"

#include <time.h>
#include <zlib.h>


// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::CollisionDetectionSystem collisionSystem;
    our::Text* currentPlayerText, *timeText, *numberOfCollectedArtifactsText;

    our::MovementRestriction movementRestriction;

    // This all relates to the speed collectable effect.
    our::SpeedCollectableInfo speed;

    // This will indicate the total number of collectable artifacts. 
    int totalNumberOfArtifacts;

    // This records the time the player took to finish the race.
    float elapsedTime = 0.0;
    void onInitialize() override {

        std::cout << "Play state type: " << type << std::endl;
        
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }
        
        our::MultipleMeshesRendererComponent *track = nullptr;
        our::CameraComponent* camera = nullptr;
        our::FreeCameraControllerComponent *controller = nullptr;
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
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
                if (light = entity->getComponent<our::LightComponent>(); light) {
                    world.setOfLights.insert(light);
                }

                if (!track) {
                    track = entity->getComponent<our::MultipleMeshesRendererComponent>();
                    if (track) {
                        if (track->nameOfMeshObject == "track") world.track.trackLength = track->getOwner()->localTransform.scale.z;
                        else track = NULL;
                    }
                }
            }
        }

        // Setting options within game-config
        if (config.contains("game-config")) {
            // Overriding the track length if it was provided as a parameter in the game config.
            world.track.trackLength = config["game-config"].value("track-length", world.track.trackLength);
            if (track) {
                track->getOwner()->localTransform.scale.z = world.track.trackLength;
                track->getOwner()->localTransform.position.z = -3.9 * world.track.trackLength;
                world.setTrackRelatedVariables(track);
            }

            // This controls whether the player is allowed all range of movement along the axes.
            if (config["game-config"].contains("movement-control")) {
                if (!config["game-config"]["movement-control"].is_object())
                    std::cerr << "ERROR: movement-control IN config.json MUST BE AN OBJECT." << std::endl;
                else {
                    movementRestriction.restrict_x = config["game-config"]["movement-control"].value("restrict-x", false);
                    movementRestriction.restrict_y = config["game-config"]["movement-control"].value("restrict-y", false);
                    movementRestriction.restrict_z = config["game-config"]["movement-control"].value("restrict-z", false);
                    movementRestriction.autoMoveForward = config["game-config"]["movement-control"].value("auto-forward-movement", false);
                }
            }
        }

        if (camera)
            // Centering the camera on the desired position.
            camera->setPosition(glm::vec3( (world.track.tracksFarLeft.x+world.track.tracksFarRight.x)/2.0, 2.0, 0.0));

        // We create the randomized artifacts.
        this->createRandomizedArtifacts();

        // We create the randomized environment.
        this->createRandomizedEnvironment();

        // Create the finish line.
        this->createMirrorOfFinishLine(track);

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

    void createMirrorOfFinishLine(our::MultipleMeshesRendererComponent* track) {

        our::Entity* finishLine = world.add();

        finishLine->localTransform.position = glm::vec3((world.track.tracksFarLeft.x + world.track.tracksFarRight.x) / 2.0, 20, world.track.tracksZFurthest);
        finishLine->localTransform.scale = glm::vec3(23, 20, 5);
        
        our::MeshRendererComponent* finishLineRenderer = finishLine->addComponent<our::MeshRendererComponent>();
        finishLineRenderer->mesh = our::AssetLoader<our::Mesh>::get("plane");
        finishLineRenderer->nameOfMeshObject = "plane";

        finishLineRenderer->material = our::AssetLoader<our::Material>::get("portal");
        finishLineRenderer->material->shader = our::AssetLoader<our::ShaderProgram>::get("textured");
        finishLineRenderer->material->pipelineState.depthTesting.enabled = true;
    }

    // This function creates a random number of collectable artifacts at random locations
    // along the race track, between a minimum and a maximum number of total artifacts.
    void createRandomizedArtifacts() {
        
        srand(time(0));
        
        int minArtifacts = 40, maxArtifacts = 100;
        totalNumberOfArtifacts = rand() % (maxArtifacts - minArtifacts + 1);
        totalNumberOfArtifacts += minArtifacts;
        float xCoordinate, zCoordinate;

        std::cout << "The random number of artifacts will be: " << totalNumberOfArtifacts << std::endl;


        for (int i = 0; i < totalNumberOfArtifacts; i++) {
            
            our::Entity* newEntity = world.add();
            newEntity->typeOfChildMesh = our::COLLECTABLE_COIN;

            zCoordinate = rand() % (long)(5.0 - world.track.tracksZFurthest + 5.0);
            zCoordinate += world.track.tracksZFurthest;

            xCoordinate = rand() % (long)(world.track.tracksFarRight.x - world.track.tracksFarLeft.x + 1.0);
            xCoordinate += world.track.tracksFarLeft.x;

            newEntity->localTransform.position = glm::vec3(xCoordinate, 5, zCoordinate);
            
            our::MeshRendererComponent *meshComponent = newEntity->addComponent<our::MeshRendererComponent>();
            meshComponent->mesh = our::AssetLoader<our::Mesh>::get("collectable");
            meshComponent->material = our::AssetLoader<our::Material>::get("moon"); 

            world.setOfSpaceArtifacts.insert(newEntity);
        }

        // Generate a speed collectable with a 1/2 probability.
        if (rand() % 2 == 0) {
            
            std::cout << "Lucky you! A speed collectable is generated" << std::endl;
            our::Entity* speedCollectable = world.add();
            speedCollectable->typeOfChildMesh = our::SPEED_COLLECTABLE;
            our::MeshRendererComponent* speedMesh = speedCollectable->addComponent<our::MeshRendererComponent>();
            speedMesh->mesh = our::AssetLoader<our::Mesh>::get("collectable");
            speedMesh->material = our::AssetLoader<our::Material>::get("greenMetal");

            zCoordinate = rand() % (long)(5.0 - world.track.tracksZFurthest + 5.0);
            zCoordinate += world.track.tracksZFurthest;

            xCoordinate = rand() % (long)(world.track.tracksFarRight.x - world.track.tracksFarLeft.x + 1.0);
            xCoordinate += world.track.tracksFarLeft.x;


            speedCollectable->localTransform.position = glm::vec3(xCoordinate, 5, zCoordinate);
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
        int minPlanets = 50, maxPlanets = 90, minY = 20, maxY = 150;
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
            scale = (rand() % (5 - 2 + 1)) + 2;
            
            // Randomizing coordinates, and looping until we make sure that the new planet
            // is of minimum required distance to all other planets.
            bool invalid = false;
            do {
                xCoordinate = (sign[rand() % 2]) * (rand() % 240);
                yCoordinate = (sign[rand() % 2]) * ((rand() % (maxY - minY + 1)+minY));
                zCoordinate = - rand() % 200;
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
            meshComponent->material = our::AssetLoader<our::Material>::get(planets[rand() % 2]);

            // Adding a movement component to the planet, that is equivalent of the planet's rotation
            // around its own axis. 
            our::MovementComponent* rotationComponent = newPlanet->addComponent<our::MovementComponent>();
            rotationComponent->angularVelocity = glm::vec3(0.0, ((rand() % 120) / 180.0) * glm::pi<float>(), 0);

            setOfCelestialOrbs.insert(newPlanet);

            // Creating a moon around the planet.
            // We create a moon around every 1 out of 3 planets.
            if (rand() % 3 > 1) {
                
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
                xCoordinate = (sign[rand() % 2]) * (rand() % 240);
                yCoordinate = (sign[rand() % 2]) * ((rand() % (maxY - minY + 1)+minY));
                zCoordinate = - rand() % 500;
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
            scale = (rand() % (7 - 4 + 1)) + 4;
            newStar->localTransform.scale = glm::vec3(scale);
            
            // Creating the MeshRenderer component, which will be a sphere,
            // and will have a material with a planet texture, that will be randomized
            // from the ones in the planets array above.
            our::MeshRendererComponent *starMesh = newStar->addComponent<our::MeshRendererComponent>();
            starMesh->mesh = our::AssetLoader<our::Mesh>::get("sphere");
            starMesh->material = our::AssetLoader<our::Material>::get("star");

            // Creating a (point) light component and adding it to the star.
            our::LightComponent *starLight = newStar->addComponent<our::LightComponent>();
            starLight->type = our::POINT; starLight->attenuation = glm::vec3(0,  0, 1); // attenuation 1/d for now
            starLight->color = glm::vec3(1.0);

            world.setOfLights.insert(starLight);

            // Adding a movement component to the star, that is equivalent of the star's rotation
            // around its own axis.
            our::MovementComponent* starRotation = newStar->addComponent<our::MovementComponent>();
            starRotation->angularVelocity = glm::vec3(0.0, ((rand() % 60) / 180.0) * glm::pi<float>(), 0);

            setOfCelestialOrbs.insert(newStar);

        }

        setOfCelestialOrbs.clear();
    }

    void onDraw(double deltaTime) override {

        speed.inEffect = false;
        
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        
        // forbiddenAccess: whether the player has tried to enter a forbidden zone.
        // forbiddenCollision: whether the player has collided with a planet. 
        bool forbiddenAccess = false, forbiddenCollision = false;
        
        // Get the camera component, and fetch a boolean whether the player
        // has tried to access a forbidden zone. (Violating any of the movement restrictions)
        // This boolean is then passed to the renderer in the render function.

        // Here, we get the new position of the camera in the updatedCameraPosition vector.
        // We don't update the position of the camera directly, we take updatedCameraPosition, 
        // check if there is any forbidden collisions first, and if there is not, we update
        // the actual camera position to be updatedCameraPosition.
        glm::vec3 updatedCameraPosition;
        our::Entity* camerasParent = cameraController.update(&world, (float)deltaTime, &updatedCameraPosition, &forbiddenAccess, movementRestriction, speed.timeSince);
        
        our::CameraComponent* camera = camerasParent->getComponent<our::CameraComponent>();
        our::FreeCameraControllerComponent* cameraControllerComponente = camerasParent->getComponent<our::FreeCameraControllerComponent>();
        if (!camera || !cameraControllerComponente)
            std::cerr << "ERROR:: MISSING CAMERA OR CONTROLLER IN PLAY STATE" << std::endl;

        // Obtain the remaining number of collectables to update the text.
        int remainingCollectables = collisionSystem.update(&world, updatedCameraPosition, getApp()->getSoundEngine(), &forbiddenCollision, &speed);

        // If a forbidden collision has not happen, update the actual camera position.
        if (!forbiddenCollision) {
            // actually updating the position of the camera.
            camera->setPosition(updatedCameraPosition);
        }
        
        if (speed.timeSince == 0.0) {
            if (speed.inEffect) {
                camera->fovY = 3.0;
                speed.timeSince = glfwGetTime();
                cameraControllerComponente->positionSensitivity = glm::vec3(10.0, 10.0, 10.0);
                speed.zAtTimeOfCollection = updatedCameraPosition.z;
                speed.pervPostprocess = renderer.postprocessInEffect;
                renderer.postprocessInEffect = "speedup";
                currentPlayerText->color = timeText->color = numberOfCollectedArtifactsText->color = glm::vec3(0, 0, 0);
            }
        } else {
            if (glfwGetTime() - speed.timeSince > 10.0) {
                camera->fovY = 1.518;
                cameraControllerComponente->positionSensitivity = glm::vec3(6.0, 6.0, 6.0);
                renderer.postprocessInEffect = speed.pervPostprocess;
                speed.timeSince = 0.0;
                speed.zAtTimeOfCollection = 100.0;
                speed.inEffect = false;
                currentPlayerText->color = timeText->color = numberOfCollectedArtifactsText->color = glm::vec3(1, 1, 1);
            }
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
        if (camera->getOwner()->localTransform.position.z <= world.track.tracksZFurthest) {
            getApp()->setPlayerStats((our::PlaystateType)type, elapsedTime, (1.0-world.setOfSpaceArtifacts.size())/totalNumberOfArtifacts);
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