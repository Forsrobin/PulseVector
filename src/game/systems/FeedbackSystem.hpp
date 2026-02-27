#pragma once

#include "engine/ecs/System.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/core/Events.hpp"
#include "engine/graphics/PostProcessManager.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <memory>

namespace game::systems {

class FeedbackSystem : public engine::ecs::ISystem {
public:
    FeedbackSystem(engine::core::EventBus& eventBus, 
                   engine::graphics::PostProcessManager& postProcess,
                   std::shared_ptr<sf::Texture> spriteMap,
                   std::shared_ptr<sf::Font> font);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    void onHitEvent(const engine::core::HitEvent& event);

    engine::core::EventBus& m_eventBus;
    engine::graphics::PostProcessManager& m_postProcess;
    std::shared_ptr<sf::Texture> m_spriteMap;
    std::shared_ptr<sf::Font> m_font;
    entt::registry* m_registry{nullptr};
};

} // namespace game::systems
