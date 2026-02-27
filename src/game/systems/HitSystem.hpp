#pragma once

#include "engine/ecs/System.hpp"
#include "engine/audio/AudioCore.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/Events.hpp"
#include <map>

namespace game::systems {

class HitSystem : public engine::ecs::ISystem {
public:
    HitSystem(engine::audio::AudioCore& audioCore, engine::core::EventBus& eventBus);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

    std::optional<int> getDirectionForKey(sf::Keyboard::Key key) const {
        auto it = m_keyToDirection.find(key);
        if (it != m_keyToDirection.end()) return it->second;
        return std::nullopt;
    }

private:
    void onKeyEvent(const engine::core::KeyEvent& event);
    
    engine::audio::AudioCore& m_audioCore;
    engine::core::EventBus& m_eventBus;
    entt::registry* m_registry{nullptr};

    std::map<sf::Keyboard::Key, int> m_keyToDirection;
    
    // Hit Windows (in seconds)
    float m_perfectWindow{0.05f};
    float m_greatWindow{0.10f};
    float m_goodWindow{0.15f};
    float m_missWindow{0.20f};
};

} // namespace game::systems
