#pragma once

#include <SFML/System/Vector2.hpp>

namespace engine::graphics::components {

struct Transform {
    sf::Vector2f position;
    sf::Vector2f scale{1.0f, 1.0f};
    float rotation{0.0f};
};

} // namespace engine::graphics::components
