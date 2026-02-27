#include "WallSystem.hpp"
#include "../components/Wall.hpp"
#include "engine/core/Events.hpp"
#include <SFML/Graphics/VertexArray.hpp>
#include <cmath>

namespace game::systems {

WallSystem::WallSystem(engine::core::Application& app) : m_app(app) {
    if (!m_wallShader.loadFromFile("shaders/default.vert", "shaders/wall.frag")) {
        fmt::print(stderr, "WallSystem: Failed to load wall shader\n");
    }
}

void WallSystem::update(entt::registry& registry, sf::Time dt) {
    float elapsed = dt.asSeconds();
    auto mousePos = m_app.getInputSystem().getMousePosition();
    sf::Vector2f relMouse = mousePos - m_center;
    float mouseDist = std::sqrt(relMouse.x * relMouse.x + relMouse.y * relMouse.y);
    float mouseAngle = std::atan2(relMouse.y, relMouse.x) * 180.f / 3.14159f;
    if (mouseAngle < 0) mouseAngle += 360.f;

    auto view = registry.view<components::Wall>();
    for (auto entity : view) {
        auto& wall = view.get<components::Wall>(entity);
        
        wall.distance -= wall.speed * elapsed;
        
        if (wall.distance < 20.f) {
            registry.destroy(entity);
            continue;
        }

        // Collision Check
        float wallThickness = 20.f;
        if (mouseDist >= wall.distance && mouseDist <= wall.distance + wallThickness) {
            float normStart = wall.startAngle;
            float normEnd = wall.startAngle + wall.sweepAngle;
            
            bool hit = false;
            if (normEnd > 360.f) {
                hit = (mouseAngle >= normStart || mouseAngle <= (normEnd - 360.f));
            } else {
                hit = (mouseAngle >= normStart && mouseAngle <= normEnd);
            }

            if (hit) {
                // Penalize player (Miss event triggers HP loss in ScoringSystem)
                m_app.getEventBus().publish(engine::core::HitEvent{engine::core::HitRating::Miss, 0.f, entt::null, mousePos});
            }
        }
    }
}

void WallSystem::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry; (void)dt;
}

void WallSystem::render(entt::registry& registry, float interpolation) {
    (void)interpolation;
    auto& target = m_app.getRenderTarget();
    auto view = registry.view<components::Wall>();

    float amplitude = m_app.getAudioCore().getAmplitude();
    float bass = m_app.getAudioCore().getBassEnergy();
    static float s_time = 0.f;
    s_time += 0.016f; // rough approximation for shader animation

    m_wallShader.setUniform("time", s_time);
    m_wallShader.setUniform("amplitude", amplitude);
    m_wallShader.setUniform("bass", bass);
    m_wallShader.setUniform("resolution", sf::Vector2f(1280.f, 720.f));

    for (auto entity : view) {
        const auto& wall = view.get<components::Wall>(entity);
        
        float wallThickness = 20.f;
        size_t points = 40; // Increased points for better quality with shaders
        sf::VertexArray va(sf::PrimitiveType::TriangleStrip, points * 2);

        sf::Color wallColor = sf::Color(255, 50, 50, 180);
        m_wallShader.setUniform("u_baseColor", sf::Glsl::Vec4(wallColor));

        for (size_t i = 0; i < points; ++i) {
            float angle = (wall.startAngle + (static_cast<float>(i) / (points - 1)) * wall.sweepAngle) * 3.14159f / 180.f;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            
            va[i * 2].position = m_center + dir * wall.distance;
            va[i * 2].color = wallColor;
            
            va[i * 2 + 1].position = m_center + dir * (wall.distance + wallThickness);
            va[i * 2 + 1].color = wallColor;
        }
        target.draw(va, &m_wallShader);
    }
}

} // namespace game::systems
