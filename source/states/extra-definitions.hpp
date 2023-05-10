#pragma once

namespace our {
    enum PlaystateType {
        USSR,
        US,
    };

    // These three flags control whether the players want their movement restricted or not.
    struct MovementRestriction {
        bool restrict_x, restrict_y, restrict_z;
    };
}