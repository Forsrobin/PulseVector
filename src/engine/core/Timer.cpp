#include "Timer.hpp"

namespace engine::core {

Timer::Timer(sf::Time fixedTimeStep)
    : m_fixedTimeStep(fixedTimeStep)
    , m_accumulator(sf::Time::Zero)
    , m_deltaTime(sf::Time::Zero) {
    m_clock.restart();
}

void Timer::update() {
    m_deltaTime = m_clock.restart();
    m_accumulator += m_deltaTime;
}

bool Timer::shouldUpdateFixed() const {
    return m_accumulator >= m_fixedTimeStep;
}

void Timer::consumeFixedUpdate() {
    m_accumulator -= m_fixedTimeStep;
}

float Timer::getInterpolationFactor() const {
    return m_accumulator.asSeconds() / m_fixedTimeStep.asSeconds();
}

sf::Time Timer::getDeltaTime() const {
    return m_deltaTime;
}

sf::Time Timer::getFixedTimeStep() const {
    return m_fixedTimeStep;
}

} // namespace engine::core
