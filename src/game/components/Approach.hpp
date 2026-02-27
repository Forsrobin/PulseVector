#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>

namespace game::components {

struct Approach {
    float startTimeSeconds;
    float targetTimeSeconds;
    float startScale{4.0f};
    float targetScale{1.0f};
    std::vector<sf::Vector2f> pathPoints;
    float approachFactor{0.f}; // 0.0 at start, 1.0 at target
};

} // namespace game::components
