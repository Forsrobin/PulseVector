#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

namespace engine::core {

struct KeyEvent {
    sf::Keyboard::Key key;
    bool pressed;
};

struct MouseMoveEvent {
    sf::Vector2f position;
};

struct MouseButtonEvent {
    sf::Mouse::Button button;
    bool pressed;
    sf::Vector2f position;
};

enum class HitRating {
    Perfect,
    Great,
    Good,
    Miss
};

struct HitEvent {
    HitRating rating;
    float offsetSeconds;
    entt::entity entity;
    sf::Vector2f position;
};

struct ScoreUpdateEvent {
    int currentScore;
    int currentCombo;
    int multiplier;
};

struct GameOverEvent {
    int finalScore;
};

} // namespace engine::core
