#pragma once

#include "../ecs/System.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace engine::graphics {

class ParticleSystem : public engine::ecs::ISystem {
public:
    explicit ParticleSystem(sf::RenderTarget& target);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    sf::RenderTarget& m_target;
    sf::VertexArray m_vertices;
};

} // namespace engine::graphics
