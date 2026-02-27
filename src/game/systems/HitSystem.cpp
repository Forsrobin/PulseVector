#include "HitSystem.hpp"
#include "../components/HitObject.hpp"
#include "engine/graphics/components/Transform.hpp"
#include <cmath>

namespace game::systems {

HitSystem::HitSystem(engine::audio::AudioCore& audioCore, engine::core::EventBus& eventBus)
    : m_audioCore(audioCore), m_eventBus(eventBus) {
    
    // Default key mapping
    m_keyToDirection[sf::Keyboard::Key::Up] = 0;
    m_keyToDirection[sf::Keyboard::Key::W] = 0;
    
    m_keyToDirection[sf::Keyboard::Key::Right] = 1;
    m_keyToDirection[sf::Keyboard::Key::D] = 1;
    
    m_keyToDirection[sf::Keyboard::Key::Down] = 2;
    m_keyToDirection[sf::Keyboard::Key::S] = 2;
    
    m_keyToDirection[sf::Keyboard::Key::Left] = 3;
    m_keyToDirection[sf::Keyboard::Key::A] = 3;

    m_eventBus.subscribe<engine::core::KeyEvent>([this](const auto& event) {
        this->onKeyEvent(event);
    });
}

void HitSystem::update(entt::registry& registry, sf::Time dt) {
    (void)dt;
    m_registry = &registry;

    if (!m_audioCore.isPlaying()) return;

    float currentAudioTime = m_audioCore.getPlaybackPosition().asSeconds();

    // Check for missed objects (passed the miss window)
    auto view = registry.view<components::HitObject>();
    for (auto entity : view) {
        const auto& hitObject = view.get<components::HitObject>(entity);
        if (currentAudioTime > hitObject.hitTimeSeconds + m_goodWindow) {
            sf::Vector2f pos(0.f, 0.f);
            if (auto* transform = registry.try_get<engine::graphics::components::Transform>(entity)) {
                pos = transform->position;
            }
            m_eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Miss, currentAudioTime - hitObject.hitTimeSeconds, entity, pos});
            registry.destroy(entity);
        }
    }
}

void HitSystem::onKeyEvent(const engine::core::KeyEvent& event) {
    if (!event.pressed || !m_registry || !m_audioCore.isPlaying()) return;

    auto it = m_keyToDirection.find(event.key);
    if (it == m_keyToDirection.end()) return;

    int pressedDir = it->second;
    float currentAudioTime = m_audioCore.getPlaybackPosition().asSeconds();

    // Find the closest hit object with the same direction
    entt::entity bestEntity = entt::null;
    float bestOffset = std::numeric_limits<float>::max();
    
    auto view = m_registry->view<components::HitObject>();
    for (auto entity : view) {
        const auto& hitObject = view.get<components::HitObject>(entity);
        
        // We only care about objects that are roughly in the window
        float offset = std::abs(currentAudioTime - hitObject.hitTimeSeconds);
        if (offset < m_missWindow) {
            if (offset < bestOffset) {
                bestOffset = offset;
                bestEntity = entity;
            }
        }
    }

    if (bestEntity != entt::null) {
        const auto& hitObject = view.get<components::HitObject>(bestEntity);
        
        engine::core::HitRating rating = engine::core::HitRating::Miss;
        
        if (pressedDir == hitObject.direction) {
            if (bestOffset <= m_perfectWindow) rating = engine::core::HitRating::Perfect;
            else if (bestOffset <= m_greatWindow) rating = engine::core::HitRating::Great;
            else if (bestOffset <= m_goodWindow) rating = engine::core::HitRating::Good;
        }

        sf::Vector2f pos(0.f, 0.f);
        if (auto* transform = m_registry->try_get<engine::graphics::components::Transform>(bestEntity)) {
            pos = transform->position;
        }

        m_eventBus.publish(engine::core::HitEvent{rating, currentAudioTime - hitObject.hitTimeSeconds, bestEntity, pos});
        m_registry->destroy(bestEntity);
    }
}

void HitSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void HitSystem::render(entt::registry& registry, float interpolation) {
    (void)registry; (void)interpolation;
}

} // namespace game::systems
