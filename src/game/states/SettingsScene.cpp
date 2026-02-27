#include "SettingsScene.hpp"
#include "MenuScene.hpp"
#include "TitleScene.hpp"
#include "engine/core/Application.hpp"
#include "engine/graphics/RenderSystem.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <fmt/core.h>
#include <algorithm>

namespace game::states {

SettingsScene::SettingsScene(engine::core::Application& app)
    : m_app(app)
    , m_volume(app.getVolume()) {
}

void SettingsScene::onInitialize(entt::registry& registry) {
    addSystem(std::make_unique<engine::graphics::RenderSystem>(m_app.getRenderTarget()));

    auto bgTex = m_app.getTextureManager().get("background_2");
    if (bgTex) {
        m_backgroundSprite.emplace(*bgTex);
        sf::Vector2u texSize = bgTex->getSize();
        m_backgroundSprite->setScale({1280.f / texSize.x, 720.f / texSize.y});
        m_backgroundSprite->setColor(sf::Color(255, 255, 255, 100));
    }

    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_titleText.emplace(*font, "SETTINGS");
        m_titleText->setCharacterSize(80);
        m_titleText->setFillColor(sf::Color::Cyan);
        auto titleBounds = m_titleText->getLocalBounds();
        m_titleText->setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
        m_titleText->setPosition({640.f, 100.f});

        m_volumeText.emplace(*font, fmt::format("Volume: {:.0f}%", m_volume));
        m_volumeText->setCharacterSize(40);
        m_volumeText->setFillColor(sf::Color::White);
        m_volumeText->setPosition({400.f, 300.f});

        m_backText.emplace(*font, "Press ESC to return");
        m_backText->setCharacterSize(30);
        m_backText->setFillColor(sf::Color(200, 200, 200));
        auto backBounds = m_backText->getLocalBounds();
        m_backText->setOrigin({backBounds.size.x / 2.f, backBounds.size.y / 2.f});
        m_backText->setPosition({640.f, 650.f});

        m_homeText.emplace(*font, "Home");
        m_homeText->setCharacterSize(40);
        m_homeText->setFillColor(sf::Color::White);
        auto homeBounds = m_homeText->getLocalBounds();
        m_homeText->setOrigin({homeBounds.size.x / 2.f, homeBounds.size.y / 2.f});
        m_homeText->setPosition({640.f, 480.f});
        m_homeBounds = sf::FloatRect({640.f - homeBounds.size.x/2.f, 480.f - homeBounds.size.y/2.f}, homeBounds.size);
    }

    m_sliderBar.setSize({400.f, 10.f});
    m_sliderBar.setFillColor(sf::Color(100, 100, 100));
    m_sliderBar.setPosition({400.f, 380.f});

    m_sliderKnob.setSize({20.f, 30.f});
    m_sliderKnob.setFillColor(sf::Color::Cyan);
    m_sliderKnob.setOrigin({10.f, 15.f});
}

void SettingsScene::update(entt::registry& registry, sf::Time dt) {
    Scene::update(registry, dt);

    auto mousePos = m_app.getInputSystem().getMousePosition();
    sf::FloatRect sliderBounds(m_sliderBar.getPosition(), m_sliderBar.getSize());
    // Expand bounds for easier grabbing
    sliderBounds.position.y -= 20.f;
    sliderBounds.size.y += 40.f;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        if (sliderBounds.contains(mousePos)) {
            m_isDragging = true;
        }
    } else {
        m_isDragging = false;
    }

    if (m_isDragging) {
        float relativeX = mousePos.x - m_sliderBar.getPosition().x;
        float percent = std::clamp(relativeX / m_sliderBar.getSize().x, 0.f, 1.f);
        m_volume = percent * 100.f;
        m_app.setVolume(m_volume);
        if (m_volumeText) {
            m_volumeText->setString(fmt::format("Volume: {:.0f}%", m_volume));
        }
    }

    if (m_homeBounds.contains(mousePos)) {
        if (m_homeText) m_homeText->setFillColor(sf::Color::Cyan);
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            m_app.transitionToScene(std::make_unique<TitleScene>(m_app));
        }
    } else {
        if (m_homeText) m_homeText->setFillColor(sf::Color::White);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        m_app.transitionToScene(std::make_unique<TitleScene>(m_app));
    }
}

void SettingsScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    if (m_backgroundSprite) target.draw(*m_backgroundSprite);

    Scene::render(registry, interpolation);

    if (m_titleText) target.draw(*m_titleText);
    if (m_volumeText) target.draw(*m_volumeText);
    if (m_homeText) target.draw(*m_homeText);
    if (m_backText) target.draw(*m_backText);

    target.draw(m_sliderBar);
    
    float knobX = m_sliderBar.getPosition().x + (m_volume / 100.f) * m_sliderBar.getSize().x;
    m_sliderKnob.setPosition({knobX, m_sliderBar.getPosition().y + 5.f});
    target.draw(m_sliderKnob);
}

} // namespace game::states
