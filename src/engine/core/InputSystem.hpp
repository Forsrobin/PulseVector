#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

#include "EventBus.hpp"

namespace engine::core {

class InputSystem {
public:
    explicit InputSystem(EventBus& eventBus);

    void processEvent(const sf::Event& event, const sf::RenderWindow& window);

    [[nodiscard]] sf::Vector2f getMousePosition() const;
    [[nodiscard]] bool isKeyPressed(sf::Keyboard::Key key) const;

private:
    EventBus& m_eventBus;
    sf::Vector2f m_mousePosition;
};

} // namespace engine::core
