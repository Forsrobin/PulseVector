#include "ParticleSystem.hpp"
#include "components/Transform.hpp"
#include "components/Particle.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

namespace engine::graphics {

ParticleSystem::ParticleSystem(sf::RenderTarget& target)
    : m_target(target)
    , m_vertices(sf::PrimitiveType::Triangles) {
}

void ParticleSystem::update(entt::registry& registry, sf::Time dt) {
    (void)registry;
    (void)dt;
}

void ParticleSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    auto view = registry.view<components::Transform, components::Particle>();
    for (auto entity : view) {
        auto& transform = view.get<components::Transform>(entity);
        auto& particle = view.get<components::Particle>(entity);

        particle.currentLifetime += dt;
        if (particle.currentLifetime >= particle.lifetime) {
            registry.destroy(entity);
            continue;
        }

        transform.position += particle.velocity * dt.asSeconds();
        transform.rotation += particle.angularVelocity * dt.asSeconds();
    }
}

void ParticleSystem::render(entt::registry& registry, float interpolation) {
    (void)interpolation;
    auto view = registry.view<components::Transform, components::Particle>();
    size_t numParticles = std::distance(view.begin(), view.end());
    m_vertices.resize(numParticles * 3); // Simple triangles for now

    size_t i = 0;
    for (auto entity : view) {
        const auto& transform = view.get<components::Transform>(entity);
        const auto& particle = view.get<components::Particle>(entity);

        float t = particle.currentLifetime.asSeconds() / particle.lifetime.asSeconds();
        float size = particle.sizeStart + (particle.sizeEnd - particle.sizeStart) * t;
        sf::Color color = particle.colorStart; // Simplified color transition

        // Basic triangle centered at transform.position
        m_vertices[i * 3 + 0].position = transform.position + sf::Vector2f(0.f, -size);
        m_vertices[i * 3 + 1].position = transform.position + sf::Vector2f(-size, size);
        m_vertices[i * 3 + 2].position = transform.position + sf::Vector2f(size, size);

        m_vertices[i * 3 + 0].color = color;
        m_vertices[i * 3 + 1].color = color;
        m_vertices[i * 3 + 2].color = color;

        i++;
    }

    m_target.draw(m_vertices);
}

} // namespace engine::graphics
