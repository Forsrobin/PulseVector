#pragma once

#include <vector>
#include <memory>
#include <entt/entt.hpp>
#include <SFML/System/Time.hpp>
#include "../ecs/System.hpp"

namespace engine::core {

class Scene {
public:
    virtual ~Scene() = default;

    virtual void onInitialize(entt::registry& registry) { (void)registry; }
    virtual void onShutdown(entt::registry& registry) { (void)registry; }

    void addSystem(std::unique_ptr<engine::ecs::ISystem> system);

    virtual void update(entt::registry& registry, sf::Time dt);
    virtual void fixedUpdate(entt::registry& registry, sf::Time dt);
    virtual void render(entt::registry& registry, float interpolation);

private:
    std::vector<std::unique_ptr<engine::ecs::ISystem>> m_systems;
};

} // namespace engine::core
