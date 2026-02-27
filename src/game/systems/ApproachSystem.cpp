#include "ApproachSystem.hpp"
#include "../components/Approach.hpp"
#include "engine/graphics/components/Transform.hpp"
#include "engine/utils/Spline.hpp"
#include <algorithm>

namespace game::systems {

ApproachSystem::ApproachSystem(engine::audio::AudioCore& audioCore)
    : m_audioCore(audioCore) {
}

void ApproachSystem::update(entt::registry& registry, sf::Time dt) {
    (void)dt;
    if (!m_audioCore.isPlaying()) return;

    float currentAudioTime = m_audioCore.getSampleTime().asSeconds();

    auto view = registry.view<engine::graphics::components::Transform, components::Approach>();
    for (auto entity : view) {
        auto& transform = view.get<engine::graphics::components::Transform>(entity);
        const auto& approach = view.get<components::Approach>(entity);

        float duration = approach.targetTimeSeconds - approach.startTimeSeconds;
        if (duration <= 0.0f) continue;

        float elapsed = currentAudioTime - approach.startTimeSeconds;
        float t = std::clamp(elapsed / duration, 0.0f, 1.0f);

        // Spline interpolation for position
        if (!approach.pathPoints.empty()) {
            transform.position = engine::utils::Spline::interpolate(approach.pathPoints, t);
        }

        // Linear interpolation for scale
        float currentScale = approach.startScale + t * (approach.targetScale - approach.startScale);
        transform.scale = {currentScale, currentScale};
    }
}

void ApproachSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void ApproachSystem::render(entt::registry& registry, float interpolation) {
    (void)registry; (void)interpolation;
}

} // namespace game::systems
