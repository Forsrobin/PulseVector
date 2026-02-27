#pragma once

#include <SFML/System/Vector2.hpp>

namespace game::components {

struct Wall {
    float distance; // Current distance from center
    float speed;    // Speed moving towards center
    float startAngle; // Start angle in degrees
    float sweepAngle; // Angle sweep in degrees
    bool isActive{true};
};

} // namespace game::components
