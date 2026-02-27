#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Color.hpp>

namespace engine::graphics::components {

struct Particle {
    sf::Vector2f velocity;
    float angularVelocity{0.0f};
    sf::Time lifetime;
    sf::Time currentLifetime;
    sf::Color colorStart{sf::Color::White};
    sf::Color colorEnd{sf::Color::White};
    float sizeStart{1.0f};
    float sizeEnd{0.0f};
};

} // namespace engine::graphics::components
