#pragma once

#include <SFML/System/Time.hpp>

namespace engine::graphics::components {

struct LifeTime {
    sf::Time remaining;
    sf::Time total;
};

} // namespace engine::graphics::components
