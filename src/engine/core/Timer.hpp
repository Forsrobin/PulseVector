#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>

namespace engine::core {

class Timer {
public:
    explicit Timer(sf::Time fixedTimeStep = sf::milliseconds(4)); // Default ~240Hz

    void update();

    [[nodiscard]] bool shouldUpdateFixed() const;
    void consumeFixedUpdate();

    [[nodiscard]] float getInterpolationFactor() const;
    [[nodiscard]] sf::Time getDeltaTime() const;
    [[nodiscard]] sf::Time getFixedTimeStep() const;

private:
    sf::Clock m_clock;
    sf::Time m_fixedTimeStep;
    sf::Time m_accumulator;
    sf::Time m_deltaTime;
};

} // namespace engine::core
