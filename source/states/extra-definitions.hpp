#pragma once

#include <glm/glm.hpp>

#include <string>
namespace our {

    // This holds whether the turn of the play state is US, or USSR. 
    enum PlaystateType {
        USSR,
        US,
    };

    // This struct holds all the movement restriction flags.
    struct MovementRestriction {
        bool restrict_x, restrict_y, restrict_z, autoMoveForward, allowMovingBackwards, allowMouse;
    };
    
    // This struct holds all other hyper parameters.
    // Currently includes: the material and the mesh of the collectable artifacts,
    // the difference between the camera position and the adjacent aircraft,
    // the density of collectables per 1.0 distance in the z-coordinate.
    struct HyperParameters {
        glm::vec3 cameraAircraftDiff;
        std::string collectablesMaterial, collectablesMesh;
        float collectableDensity;
    };

    // This super struct holds all structs related to the game configuration.
    struct GameConfig {
        MovementRestriction movementRestriction;
        HyperParameters hyperParametrs;
    };

    // This holds the info related to the speed collectable.
    struct SpeedCollectableInfo {
        
        // This indicates whether the speed mode is in effect or not.
        bool inEffect = false;

        // This holds the time since the special speed collectable was collected.
        // This is used for epxiring the effect after X seconds.
        float timeSince = 0.0;
        
        // This holds the z coordinate at the time of collection of the speed collectable.
        float zAtTimeOfCollection = 100.0;

        // The previous postprocess effect that was applied before collecting
        // the speedup collectable. Used to return to it after the effect ends.
        std::string pervPostprocess;
    };
}