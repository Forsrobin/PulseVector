#pragma once

#include "engine/ecs/System.hpp"
#include "engine/core/Application.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace game::systems {

class WallSystem : public engine::ecs::ISystem {
public:
    explicit WallSystem(engine::core::Application& app);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    engine::core::Application& m_app;
    sf::Vector2f m_center{640.f, 360.f};
};

} // namespace game::systems
