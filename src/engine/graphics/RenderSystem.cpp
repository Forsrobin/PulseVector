#include "RenderSystem.hpp"
#include "components/Transform.hpp"
#include "components/Renderable.hpp"
#include "components/TextRenderable.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <vector>
#include <algorithm>

namespace engine::graphics {

RenderSystem::RenderSystem(sf::RenderTarget& target)
    : m_target(target) {
}

void RenderSystem::update(entt::registry& registry, sf::Time dt) {
    (void)registry;
    (void)dt;
}

void RenderSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry;
    (void)dt;
}

void RenderSystem::render(entt::registry& registry, float interpolation) {
    (void)interpolation;
    
    // Render sprites
    {
        auto view = registry.view<components::Transform, components::Renderable>();
        std::vector<entt::entity> sortedEntities(view.begin(), view.end());
        std::sort(sortedEntities.begin(), sortedEntities.end(), [&](entt::entity a, entt::entity b) {
            return view.get<components::Renderable>(a).zIndex < view.get<components::Renderable>(b).zIndex;
        });

        for (auto entity : sortedEntities) {
            const auto& transform = view.get<components::Transform>(entity);
            const auto& renderable = view.get<components::Renderable>(entity);

            if (renderable.texture) {
                sf::Sprite sprite(*renderable.texture, renderable.textureRect);
                sprite.setPosition(transform.position); 
                sprite.setRotation(sf::degrees(transform.rotation));
                sprite.setScale(transform.scale);
                sprite.setColor(renderable.color);
                m_target.draw(sprite);
            }
        }
    }

    // Render text
    {
        auto view = registry.view<components::Transform, components::TextRenderable>();
        for (auto entity : view) {
            const auto& transform = view.get<components::Transform>(entity);
            const auto& textRenderable = view.get<components::TextRenderable>(entity);

            if (textRenderable.font) {
                sf::Text text(*textRenderable.font, textRenderable.text, textRenderable.characterSize);
                text.setPosition(transform.position);
                text.setFillColor(textRenderable.color);
                // Center text
                auto bounds = text.getLocalBounds();
                text.setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
                m_target.draw(text);
            }
        }
    }
}

} // namespace engine::graphics
