#include "Scene.hpp"

namespace engine::core {

void Scene::addSystem(std::unique_ptr<engine::ecs::ISystem> system) {
    m_systems.push_back(std::move(system));
}

void Scene::update(entt::registry& registry, sf::Time dt) {
    for (auto& system : m_systems) {
        system->update(registry, dt);
    }
}

void Scene::fixedUpdate(entt::registry& registry, sf::Time dt) {
    for (auto& system : m_systems) {
        system->fixedUpdate(registry, dt);
    }
}

void Scene::render(entt::registry& registry, float interpolation) {
    for (auto& system : m_systems) {
        system->render(registry, interpolation);
    }
}

} // namespace engine::core
