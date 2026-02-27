#pragma once

#include "engine/ecs/System.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/Events.hpp"

namespace game::systems {

class ScoringSystem : public engine::ecs::ISystem {
public:
    explicit ScoringSystem(engine::core::EventBus& eventBus);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

    [[nodiscard]] int getScore() const { return m_score; }
    [[nodiscard]] int getCombo() const { return m_combo; }

private:
    void onHitEvent(const engine::core::HitEvent& event);

    engine::core::EventBus& m_eventBus;
    int m_score{0};
    int m_combo{0};
    int m_maxCombo{0};
    float m_health{100.f};
    bool m_isGameOver{false};
};

} // namespace game::systems
