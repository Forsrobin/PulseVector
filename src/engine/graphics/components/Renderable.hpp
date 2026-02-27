#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Color.hpp>
#include <memory>

namespace engine::graphics::components {

struct Renderable {
    std::shared_ptr<sf::Texture> texture;
    sf::IntRect textureRect;
    sf::Color color{sf::Color::White};
    int zIndex{0};
};

} // namespace engine::graphics::components
