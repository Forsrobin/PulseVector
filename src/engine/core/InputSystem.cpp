#include "InputSystem.hpp"
#include "Events.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

namespace engine::core {

InputSystem::InputSystem(EventBus& eventBus)
    : m_eventBus(eventBus)
    , m_mousePosition(0.f, 0.f) {
}

void InputSystem::processEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
        m_eventBus.publish(KeyEvent{keyPress->code, true});
    } else if (const auto* keyRelease = event.getIf<sf::Event::KeyReleased>()) {
        m_eventBus.publish(KeyEvent{keyRelease->code, false});
    } else if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
        m_mousePosition = window.mapPixelToCoords(mouseMove->position);
        m_eventBus.publish(MouseMoveEvent{m_mousePosition});
    } else if (const auto* mouseButtonPress = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f pos = window.mapPixelToCoords(mouseButtonPress->position);
        m_eventBus.publish(MouseButtonEvent{mouseButtonPress->button, true, pos});
    } else if (const auto* mouseButtonRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f pos = window.mapPixelToCoords(mouseButtonRelease->position);
        m_eventBus.publish(MouseButtonEvent{mouseButtonRelease->button, false, pos});
    }
}

sf::Vector2f InputSystem::getMousePosition() const {
    return m_mousePosition;
}

bool InputSystem::isKeyPressed(sf::Keyboard::Key key) const {
    return sf::Keyboard::isKeyPressed(key);
}

} // namespace engine::core
