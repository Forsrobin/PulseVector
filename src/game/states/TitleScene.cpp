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
    fmt::print("TitleScene: Initializing...\n");
    addSystem(std::make_unique<engine::graphics::RenderSystem>(m_app.getRenderTarget()));

    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_titleText.emplace(*font, "PULSE VECTOR");
        m_titleText->setCharacterSize(100);
        m_titleText->setFillColor(sf::Color::Cyan);
        auto titleBounds = m_titleText->getLocalBounds();
        m_titleText->setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
        m_titleText->setPosition({640.f, 300.f});

        m_promptText.emplace(*font, "Press SPACE to Start");
        m_promptText->setCharacterSize(40);
        m_promptText->setFillColor(sf::Color::White);
        auto promptBounds = m_promptText->getLocalBounds();
        m_promptText->setOrigin({promptBounds.size.x / 2.f, promptBounds.size.y / 2.f});
        m_promptText->setPosition({640.f, 500.f});
    } else {
        fmt::print(stderr, "TitleScene: Failed to load font!\n");
    }
}

void TitleScene::update(entt::registry& registry, sf::Time dt) {
    Scene::update(registry, dt);

    m_pulsateTimer += dt.asSeconds();
    if (m_promptText) {
        float alpha = (std::sin(m_pulsateTimer * 4.f) * 0.5f + 0.5f) * 255.f;
        m_promptText->setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        fmt::print("TitleScene: Space pressed, switching to MenuScene\n");
        m_app.transitionToScene(std::make_unique<MenuScene>(m_app));
    }
}

void TitleScene::render(entt::registry& registry, float interpolation) {
    Scene::render(registry, interpolation);

    auto& target = m_app.getRenderTarget();
    if (m_titleText) target.draw(*m_titleText);
    if (m_promptText) target.draw(*m_promptText);
}

} // namespace game::states
