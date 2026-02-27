#include "TitleScene.hpp"
#include "MenuScene.hpp"
#include "engine/core/Application.hpp"
#include "engine/graphics/RenderSystem.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <fmt/core.h>
#include <cmath>

namespace game::states {

TitleScene::TitleScene(engine::core::Application& app)
    : m_app(app) {
}

void TitleScene::onInitialize(entt::registry& registry) {
    // Background setup
    auto bgTex = m_app.getTextureManager().get("background_1");
    if (bgTex) {
        m_backgroundSprite.emplace(*bgTex);
        sf::Vector2u texSize = bgTex->getSize();
        m_backgroundSprite->setScale({1280.f / texSize.x, 720.f / texSize.y});
        m_backgroundSprite->setColor(sf::Color(255, 255, 255, 180)); // Slightly dim to make UI pop
    }

    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_titleText.emplace(*font, "PULSE VECTOR");
        m_titleText->setCharacterSize(120);
        m_titleText->setFillColor(sf::Color::Cyan);
        auto titleBounds = m_titleText->getLocalBounds();
        m_titleText->setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
        m_titleText->setPosition({640.f, 300.f});

        m_promptText.emplace(*font, "Press SPACE to Start");
        m_promptText->setCharacterSize(30);
        m_promptText->setFillColor(sf::Color::White);
        auto promptBounds = m_promptText->getLocalBounds();
        m_promptText->setOrigin({promptBounds.size.x / 2.f, promptBounds.size.y / 2.f});
        m_promptText->setPosition({640.f, 550.f});
    }

    // Initialize decorative circles
    for (int i = 0; i < 15; ++i) {
        m_decorations.push_back({
            {static_cast<float>(rand() % 1280), static_cast<float>(rand() % 720)},
            static_cast<float>(20 + rand() % 60),
            sf::Color(0, 255, 255, static_cast<std::uint8_t>(20 + rand() % 40)),
            static_cast<float>(10 + rand() % 40),
            static_cast<float>(rand() % 100) / 10.f
        });
    }
}

void TitleScene::update(entt::registry& registry, sf::Time dt) {
    Scene::update(registry, dt);

    float elapsed = dt.asSeconds();
    m_pulsateTimer += elapsed;

    if (m_titleText) {
        float tilt = std::sin(m_pulsateTimer * 2.f) * 5.f;
        m_titleText->setRotation(sf::degrees(tilt));
        float scale = 1.0f + std::sin(m_pulsateTimer * 1.5f) * 0.05f;
        m_titleText->setScale({scale, scale});
    }

    if (m_promptText) {
        float alpha = (std::sin(m_pulsateTimer * 4.f) * 0.5f + 0.5f) * 255.f;
        m_promptText->setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
    }

    // Update floating decorations
    for (auto& dec : m_decorations) {
        dec.position.y -= dec.speed * elapsed;
        if (dec.position.y + dec.radius < 0) {
            dec.position.y = 720 + dec.radius;
            dec.position.x = static_cast<float>(rand() % 1280);
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        m_app.transitionToScene(std::make_unique<MenuScene>(m_app));
    }
}

void TitleScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    
    // Draw background texture
    if (m_backgroundSprite) {
        target.draw(*m_backgroundSprite);
    }
    
    // Animated Background Grid
    sf::VertexArray grid(sf::PrimitiveType::Lines);
    sf::Color gridColor(0, 100, 255, 40);
    float spacing = 60.f;
    float scrollOffset = std::fmod(m_pulsateTimer * 20.f, spacing);

    for (float x = -scrollOffset; x <= 1280; x += spacing) {
        grid.append(sf::Vertex({x, 0}, gridColor));
        grid.append(sf::Vertex({x, 720}, gridColor));
    }
    for (float y = -scrollOffset; y <= 720; y += spacing) {
        grid.append(sf::Vertex({0, y}, gridColor));
        grid.append(sf::Vertex({1280, y}, gridColor));
    }
    target.draw(grid);

    // Draw Decorations
    for (const auto& dec : m_decorations) {
        float xOffset = std::sin(m_pulsateTimer + dec.phase) * 20.f;
        sf::CircleShape circle(dec.radius);
        circle.setOrigin({dec.radius, dec.radius});
        circle.setPosition({dec.position.x + xOffset, dec.position.y});
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(2.f);
        circle.setOutlineColor(dec.color);
        target.draw(circle);
    }

    Scene::render(registry, interpolation);

    if (m_titleText) target.draw(*m_titleText);
    if (m_promptText) target.draw(*m_promptText);
}

} // namespace game::states
