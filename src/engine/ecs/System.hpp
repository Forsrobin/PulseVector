#pragma once

#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>

namespace engine::ecs {

class ISystem {
public:
    virtual ~ISystem() = default;

    virtual void update(entt::registry& registry, sf::Time dt) = 0;
    virtual void fixedUpdate(entt::registry& registry, sf::Time dt) = 0;
    virtual void render(entt::registry& registry, float interpolation) = 0;
};

} // namespace engine::ecs
