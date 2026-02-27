#include "ScoringSystem.hpp"
#include <algorithm>

namespace game::systems {

ScoringSystem::ScoringSystem(engine::core::EventBus& eventBus)
    : m_eventBus(eventBus) {
    m_eventBus.subscribe<engine::core::HitEvent>([this](const auto& event) {
        this->onHitEvent(event);
    });
}

void ScoringSystem::onHitEvent(const engine::core::HitEvent& event) {
    if (m_isGameOver) return;

    int points = 0;
    bool resetCombo = false;
    float healthChange = 0.f;

    switch (event.rating) {
        case engine::core::HitRating::Perfect:
            points = 300;
            m_combo++;
            healthChange = 2.f;
            break;
        case engine::core::HitRating::Great:
            points = 100;
            m_combo++;
            healthChange = 1.f;
            break;
        case engine::core::HitRating::Good:
            points = 50;
            m_combo++;
            healthChange = 0.5f;
            break;
        case engine::core::HitRating::Miss:
            points = 0;
            resetCombo = true;
            healthChange = -15.f;
            break;
    }

    if (resetCombo) {
        m_combo = 0;
    }

    m_health = std::clamp(m_health + healthChange, 0.f, 100.f);

    m_maxCombo = std::max(m_maxCombo, m_combo);
    
    // Multiplier logic: 1x for 0-9 combo, 2x for 10-19, etc.
    int multiplier = 1 + (m_combo / 10);
    m_score += points * multiplier;

    m_eventBus.publish(engine::core::ScoreUpdateEvent{m_score, m_combo, multiplier});

    if (m_health <= 0.f && !m_isGameOver) {
        m_isGameOver = true;
        m_eventBus.publish(engine::core::GameOverEvent{m_score});
    }
}

void ScoringSystem::update(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void ScoringSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void ScoringSystem::render(entt::registry& registry, float interpolation) {
    (void)registry; (void)interpolation;
}

} // namespace game::systems
