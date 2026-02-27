#include "FeedbackSystem.hpp"
#include "engine/graphics/components/Transform.hpp"
#include "engine/graphics/components/Renderable.hpp"
#include "engine/graphics/components/TextRenderable.hpp"
#include "engine/graphics/components/LifeTime.hpp"
#include "engine/graphics/components/Velocity.hpp"
#include "engine/graphics/components/Particle.hpp"
#include <cstdint>
#include <cmath>

namespace game::systems {

FeedbackSystem::FeedbackSystem(engine::core::EventBus& eventBus, 
                               engine::graphics::PostProcessManager& postProcess,
                               std::shared_ptr<sf::Texture> spriteMap,
                               std::shared_ptr<sf::Font> font)
    : m_eventBus(eventBus), m_postProcess(postProcess), m_spriteMap(std::move(spriteMap)), m_font(std::move(font)) {
    
    m_eventBus.subscribe<engine::core::HitEvent>([this](const auto& event) {
        this->onHitEvent(event);
    });
}

void FeedbackSystem::update(entt::registry& registry, sf::Time dt) {
    m_registry = &registry;

    // Update lifetimes and movement
    auto view = registry.view<engine::graphics::components::Transform, 
                             engine::graphics::components::LifeTime, 
                             engine::graphics::components::Velocity>();
    
    for (auto entity : view) {
        auto& transform = view.get<engine::graphics::components::Transform>(entity);
        auto& lifetime = view.get<engine::graphics::components::LifeTime>(entity);
        const auto& velocity = view.get<engine::graphics::components::Velocity>(entity);

        lifetime.remaining -= dt;
        if (lifetime.remaining <= sf::Time::Zero) {
            registry.destroy(entity);
            continue;
        }

        // Move
        transform.position += velocity.value * dt.asSeconds();

        // Fade out
        float alphaFactor = lifetime.remaining.asSeconds() / lifetime.total.asSeconds();
        std::uint8_t alpha = static_cast<std::uint8_t>(255 * alphaFactor);

        if (auto* renderable = registry.try_get<engine::graphics::components::Renderable>(entity)) {
            renderable->color.a = alpha;
        }
        if (auto* textRenderable = registry.try_get<engine::graphics::components::TextRenderable>(entity)) {
            textRenderable->color.a = alpha;
        }
    }
}

void FeedbackSystem::onHitEvent(const engine::core::HitEvent& event) {
    if (!m_registry) return;

    // Trigger screen shake on successful hits
    if (event.rating != engine::core::HitRating::Miss) {
        float shakeAmount = 0.f;
        switch (event.rating) {
            case engine::core::HitRating::Perfect: shakeAmount = 8.f; break;
            case engine::core::HitRating::Great:   shakeAmount = 4.f; break;
            case engine::core::HitRating::Good:    shakeAmount = 2.f; break;
            default: break;
        }
        m_postProcess.addShake(shakeAmount, sf::seconds(0.15f));
    }

    // rating visual
    auto feedbackEntity = m_registry->create();
    m_registry->emplace<engine::graphics::components::Transform>(feedbackEntity, event.position);
    m_registry->emplace<engine::graphics::components::LifeTime>(feedbackEntity, sf::seconds(0.8f), sf::seconds(0.8f));
    m_registry->emplace<engine::graphics::components::Velocity>(feedbackEntity, sf::Vector2f(0.f, -50.f));

    if (event.rating == engine::core::HitRating::Miss) {
        if (m_spriteMap) {
            engine::graphics::components::Renderable renderable;
            renderable.texture = m_spriteMap;
            // "MISS!" text is Row 1, Col 0-2 (3 tiles wide)
            renderable.textureRect = sf::IntRect({0, 32}, {96, 32});
            renderable.zIndex = 20;
            m_registry->emplace<engine::graphics::components::Renderable>(feedbackEntity, renderable);
        }
    } else {
        // Particle burst for successful hits
        int particleCount = 0;
        sf::Color startColor = sf::Color::White;
        
        switch (event.rating) {
            case engine::core::HitRating::Perfect: 
                particleCount = 20; 
                startColor = sf::Color::Cyan;
                break;
            case engine::core::HitRating::Great: 
                particleCount = 12; 
                startColor = sf::Color::Green;
                break;
            case engine::core::HitRating::Good: 
                particleCount = 6; 
                startColor = sf::Color::Yellow;
                break;
            default: break;
        }

        for (int i = 0; i < particleCount; ++i) {
            auto pEntity = m_registry->create();
            m_registry->emplace<engine::graphics::components::Transform>(pEntity, event.position);
            
            float angle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f * 3.14159f;
            float speed = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 150.f + 50.f;
            sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

            m_registry->emplace<engine::graphics::components::Particle>(pEntity, 
                velocity, 
                0.f, 
                sf::seconds(0.5f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.5f),
                sf::Time::Zero,
                startColor,
                sf::Color(startColor.r, startColor.g, startColor.b, 0),
                4.f,
                0.f
            );
        }

        if (m_font) {
            engine::graphics::components::TextRenderable textRenderable;
            textRenderable.font = m_font;
            textRenderable.zIndex = 20;
            
            switch (event.rating) {
                case engine::core::HitRating::Perfect:
                    textRenderable.text = "PERFECT!";
                    textRenderable.color = sf::Color::Cyan;
                    break;
                case engine::core::HitRating::Great:
                    textRenderable.text = "GREAT";
                    textRenderable.color = sf::Color::Green;
                    break;
                case engine::core::HitRating::Good:
                    textRenderable.text = "GOOD";
                    textRenderable.color = sf::Color::Yellow;
                    break;
                default: break;
            }
            m_registry->emplace<engine::graphics::components::TextRenderable>(feedbackEntity, textRenderable);
        }
    }
}

void FeedbackSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void FeedbackSystem::render(entt::registry& registry, float interpolation) {
    (void)registry; (void)interpolation;
}

} // namespace game::systems
