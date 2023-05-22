#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem {
        Application* app; // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent 
        
        // The additional two arguments: updatedPosition, and forbiddenAccess are used to return values.
        // forbiddenAccess: returns whether the camera has tried to enter a forbidden zone, so that we may
        // draw a red screen indicating that the zone is forbidden.
        // updatedPosition: the updated position of the camera.
        Entity* update(World* world, float deltaTime, glm::vec3* updatedPosition, bool *forbiddenAccess, our::GameConfig gameConfig, float timeSinceSpeedCollected) {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            CameraComponent* camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<FreeCameraControllerComponent>();
                if(camera && controller) break;
            }
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if(!(camera && controller)) return NULL;
            // Get the entity that we found via getOwner of camera (we could use controller->getOwner())
            Entity* entity = camera->getOwner();

            // If the left mouse button is pressed, we lock and hide the mouse. This common in First Person Games.
            if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked){
                app->getMouse().lockMouse(app->getWindow());
                mouse_locked = true;
            // If the left mouse button is released, we unlock and unhide the mouse.
            } else if(!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked) {
                app->getMouse().unlockMouse(app->getWindow());
                mouse_locked = false;
            }

            // We get a reference to the entity's position and rotation
            glm::vec3& position = entity->localTransform.position;
            glm::vec3& rotation = entity->localTransform.rotation;

            // If the left mouse button is pressed, we get the change in the mouse location
            // and use it to update the camera rotation
            if (gameConfig.movementRestriction.allowMouse) {
                if(app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1)){
                    glm::vec2 delta = app->getMouse().getMouseDelta();
                    rotation.x -= delta.y * controller->rotationSensitivity; // The y-axis controls the pitch
                    rotation.y -= delta.x * controller->rotationSensitivity; // The x-axis controls the yaw
                }
            }

            // We prevent the pitch from exceeding a certain angle from the XZ plane to prevent gimbal locks
            if(rotation.x < -glm::half_pi<float>() * 0.99f) rotation.x = -glm::half_pi<float>() * 0.99f;
            if(rotation.x >  glm::half_pi<float>() * 0.99f) rotation.x  = glm::half_pi<float>() * 0.99f;
            // This is not necessary, but whenever the rotation goes outside the 0 to 2*PI range, we wrap it back inside.
            // This could prevent floating point error if the player rotates in single direction for an extremely long time. 
            rotation.y = glm::wrapAngle(rotation.y);

            // We update the camera fov based on the mouse wheel scrolling amount
            float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            fov = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f); // We keep the fov in the range 0.01*PI to 0.99*PI
            camera->fovY = fov;

            // We get the camera model matrix (relative to its parent) to compute the front, up and right directions
            glm::mat4 matrix = entity->localTransform.toMat4();

            glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, -1, 0)),
                      up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)), 
                      right = glm::vec3(matrix * glm::vec4(1, 0, 0, 0));

            glm::vec3 current_sensitivity = controller->positionSensitivity;
            // If the LEFT SHIFT key is pressed, OR we are in super speed mode,
            // we multiply the position sensitivity by the speed up factor
            if((timeSinceSpeedCollected != 0.0) || app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT)) current_sensitivity *= controller->speedupFactor;

            glm::vec3 tempPosition(position);
            *updatedPosition = position;
            
            // We change the camera position based on the keys WASD/QE
            // S & W moves the player back and forth

            if (gameConfig.movementRestriction.autoMoveForward) {
                tempPosition += front * (deltaTime * current_sensitivity.z); 
            } else {
                if (timeSinceSpeedCollected != 0.0) tempPosition += front * (deltaTime * current_sensitivity.z);
                else {
                    if(app->getKeyboard().isPressed(GLFW_KEY_W)) tempPosition += front * (deltaTime * current_sensitivity.z);
                    
                    if (gameConfig.movementRestriction.allowMovingBackwards)
                        if(app->getKeyboard().isPressed(GLFW_KEY_S)) tempPosition -= front * (deltaTime * current_sensitivity.z);
                }
            }

            // Q & E moves the player up and down
            if(app->getKeyboard().isPressed(GLFW_KEY_Q)) tempPosition += up * (deltaTime * current_sensitivity.y);
            if(app->getKeyboard().isPressed(GLFW_KEY_E)) tempPosition -= up * (deltaTime * current_sensitivity.y);
            // A & D moves the player left or right 
            if(app->getKeyboard().isPressed(GLFW_KEY_D)) {
                tempPosition +=  right * (deltaTime * current_sensitivity.x);
                if (world->airCraftEntity)
                   world->airCraftEntity->localTransform.rotation = glm::vec3(0.0, 0.0, -0.3);
            } else if (app->getKeyboard().justReleased(GLFW_KEY_D)) {
                if (world->airCraftEntity)
                    world->airCraftEntity->localTransform.rotation = glm::vec3(0.0, 0.0, 0.0);
            }

            if(app->getKeyboard().isPressed(GLFW_KEY_A)) {
                tempPosition -=  right * (deltaTime * current_sensitivity.x);
                if (world->airCraftEntity)
                    world->airCraftEntity->localTransform.rotation = glm::vec3(0.0, 0.0, 0.3);
            } else if (app->getKeyboard().justReleased(GLFW_KEY_A)) {
                if (world->airCraftEntity)
                    world->airCraftEntity->localTransform.rotation = glm::vec3(0.0, 0.0, 0.0);
            }


            // Restricting the plane movement along the x-axis, so that it can't go out of the track.
            if (gameConfig.movementRestriction.restrict_x) {

                // Obtaining the would-be position of the camera (eye) in the world coordinate, to compare with the far left point
                // of the track, and the far right point.

                // Updating the camera's model matrix with the would-be position (of the entity) to calculate the would-be
                // position of the eye if the model matrix were to be updated.
                camera->setPosition(tempPosition);

                glm::vec4 tempEyeWorld = camera->getOwner()->getLocalToWorldMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0);
                
                // Resetting the camera's model matrix after calculating tempEyeWorld. 
                camera->setPosition(position);

                if (tempEyeWorld.x < world->track.tracksFarLeft.x || tempEyeWorld.x > world->track.tracksFarRight.x)
                    *forbiddenAccess = true;
                else
                    updatedPosition->x = tempPosition.x;
            } else
                updatedPosition->x = tempPosition.x;

            // Restricting the plane movement along the y-axis, so that it can't go below the track.
            if (gameConfig.movementRestriction.restrict_y) {
                if (tempPosition.y+gameConfig.hyperParametrs.cameraAircraftDiff.y < 1)
                    *forbiddenAccess = true;
                else
                    updatedPosition->y = tempPosition.y;
            } else
                updatedPosition->y = tempPosition.y;

            // Restricting the plane movement along the z-axis, so that it can't go back behind the start line.
            if (gameConfig.movementRestriction.restrict_z) {
                if (tempPosition.z+gameConfig.hyperParametrs.cameraAircraftDiff.z > 4)
                    *forbiddenAccess = true;
                else
                    updatedPosition->z = tempPosition.z;
            } else
                updatedPosition->z = tempPosition.z;
            
            //std::cout << glm::to_string(updatedPosition) << std::endl;

            return camera->getOwner();
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit(){
            if(mouse_locked) {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }

    };

}
