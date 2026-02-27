#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <memory>

namespace engine::graphics::components {

struct TextRenderable {
    std::shared_ptr<sf::Font> font;
    std::string text;
    unsigned int characterSize{30};
    sf::Color color{sf::Color::White};
    int zIndex{10}; // Usually UI/Feedback is on top
};

} // namespace engine::graphics::components
