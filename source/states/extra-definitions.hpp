#pragma once

#include <string>
namespace our {
    enum PlaystateType {
        USSR,
        US,
    };

    // These three flags control whether the players want their movement restricted or not.
    struct MovementRestriction {
        bool restrict_x, restrict_y, restrict_z, autoMoveForward, allowMovingBackwards;
    };

    struct SpeedCollectableInfo {
        
        // This indicates whether the speed mode is in effect or not.
        bool inEffect = false;

        // This holds the time since the special speed collectable was collected.
        // This is used for epxiring the effect after X seconds.
        float timeSince = 0.0;
        
        // This holds the z coordinate at the time of collection of the speed collectable.
        float zAtTimeOfCollection = 100.0;

        std::string pervPostprocess;
    };
}