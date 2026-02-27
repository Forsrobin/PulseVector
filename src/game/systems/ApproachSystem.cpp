#include "ApproachSystem.hpp"
#include "../components/Approach.hpp"
#include "engine/graphics/components/Transform.hpp"
#include "engine/graphics/components/Particle.hpp"
#include "engine/graphics/components/Renderable.hpp"
#include "engine/graphics/components/TextRenderable.hpp"
#include "engine/utils/Spline.hpp"
#include <algorithm>

namespace game::systems {

ApproachSystem::ApproachSystem(engine::audio::AudioCore& audioCore, engine::utils::ObjectPool& pool)
    : m_audioCore(audioCore), m_pool(pool) {
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
        
        // Update approach factor
        auto& mutableApproach = const_cast<components::Approach&>(approach);
        mutableApproach.approachFactor = t;

        // Spline interpolation for position
        if (!approach.pathPoints.empty()) {
            transform.position = engine::utils::Spline::interpolate(approach.pathPoints, t);
        }

        // Linear interpolation for scale
        float currentScale = approach.startScale + t * (approach.targetScale - approach.startScale);
        transform.scale = {currentScale, currentScale};

        // ALPHA FADE: Notes start transparent and reach full opacity at t=0.3
        std::uint8_t alpha = static_cast<std::uint8_t>(std::clamp(t / 0.3f, 0.0f, 1.0f) * 255.f);
        
        if (auto* renderable = registry.try_get<engine::graphics::components::Renderable>(entity)) {
            renderable->color.a = alpha;
        }

        // TRAIL PARTICLES: Spawn a particle occasionally
        if (t > 0.1f && t < 0.95f && (rand() % 5 == 0)) {
            auto pEntity = m_pool.acquire();
            auto& pTransform = registry.get_or_emplace<engine::graphics::components::Transform>(pEntity);
            pTransform.position = transform.position;
            pTransform.scale = {0.5f, 0.5f};
            
            auto& pComp = registry.get_or_emplace<engine::graphics::components::Particle>(pEntity);
            pComp.velocity = { (rand() % 40 - 20.f), (rand() % 40 - 20.f) };
            pComp.lifetime = sf::seconds(0.3f);
            pComp.currentLifetime = sf::Time::Zero;
            pComp.colorStart = sf::Color(0, 255, 255, 100);
            pComp.colorEnd = sf::Color(0, 100, 255, 0);
            pComp.sizeStart = 3.f;
            pComp.sizeEnd = 0.f;
        }
    }
}

void ApproachSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void ApproachSystem::render(entt::registry& registry, float interpolation) {
    (void)registry; (void)interpolation;
}

} // namespace game::systems
