#include "TitleScene.hpp"
#include "MenuScene.hpp"
#include "SettingsScene.hpp"
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
        if (texSize.x > 0 && texSize.y > 0) {
            m_backgroundSprite->setScale({1280.f / texSize.x, 720.f / texSize.y});
        }
        m_backgroundSprite->setColor(sf::Color(255, 255, 255, 180)); // Slightly dim to make UI pop
    }

    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_titleText.emplace(*font, "PULSE VECTOR");
        m_titleText->setCharacterSize(140);
        m_titleText->setFillColor(sf::Color::Cyan);
        auto titleBounds = m_titleText->getLocalBounds();
        m_titleText->setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
        m_titleText->setPosition({640.f, 200.f});
    }

    m_options.push_back({"PLAY", {}, 1.0f, 0.f});
    m_options.push_back({"SETTINGS", {}, 1.0f, 0.f});
    m_options.push_back({"EXIT", {}, 1.0f, 0.f});

    // Load hover sound
    auto soundBuffer = m_app.getSoundManager().get("hover");
    if (soundBuffer) {
        m_hoverSound.emplace(*soundBuffer);
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
        float tilt = std::sin(m_pulsateTimer * 1.5f) * 3.f;
        m_titleText->setRotation(sf::degrees(tilt));
        float scale = 1.0f + std::sin(m_pulsateTimer * 1.2f) * 0.03f;
        m_titleText->setScale({scale, scale});
    }

    // Navigation
    static bool upPressed = false;
    static bool downPressed = false;
    static bool enterPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        if (!upPressed) {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_options.size())) % m_options.size();
            if (m_hoverSound) m_hoverSound->play();
            upPressed = true;
        }
    } else { upPressed = false; }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        if (!downPressed) {
            m_selectedIndex = (m_selectedIndex + 1) % m_options.size();
            if (m_hoverSound) m_hoverSound->play();
            downPressed = true;
        }
    } else { downPressed = false; }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
        if (!enterPressed) {
            auto& opt = m_options[m_selectedIndex];
            if (opt.label == "PLAY") m_app.transitionToScene(std::make_unique<MenuScene>(m_app));
            else if (opt.label == "SETTINGS") m_app.transitionToScene(std::make_unique<SettingsScene>(m_app));
            else if (opt.label == "EXIT") m_app.stop();
            enterPressed = true;
        }
    } else { enterPressed = false; }

    // Mouse
    auto mousePos = m_app.getInputSystem().getMousePosition();
    for (int i = 0; i < static_cast<int>(m_options.size()); ++i) {
        if (m_options[i].bounds.contains(mousePos)) {
            if (m_selectedIndex != i) {
                m_selectedIndex = i;
                if (m_hoverSound) m_hoverSound->play();
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                auto& opt = m_options[i];
                if (opt.label == "PLAY") m_app.transitionToScene(std::make_unique<MenuScene>(m_app));
                else if (opt.label == "SETTINGS") m_app.transitionToScene(std::make_unique<SettingsScene>(m_app));
                else if (opt.label == "EXIT") m_app.stop();
            }
        }
    }

    // Interpolation
    for (int i = 0; i < static_cast<int>(m_options.size()); ++i) {
        float targetScale = (i == m_selectedIndex) ? 1.2f : 1.0f;
        float targetOffset = (i == m_selectedIndex) ? 50.f : 0.f;
        m_options[i].scale += (targetScale - m_options[i].scale) * 10.f * elapsed;
        m_options[i].offset += (targetOffset - m_options[i].offset) * 10.f * elapsed;
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

    auto font = m_app.getFontManager().get("default");
    if (font) {
        float startX = 200.f;
        float startY = 400.f;
        float spacingY = 100.f;
        float diagonalStepX = 40.f;

        for (int i = 0; i < static_cast<int>(m_options.size()); ++i) {
            auto& opt = m_options[i];
            float posX = startX + (i * diagonalStepX) + opt.offset;
            float posY = startY + (i * spacingY);
            float width = 400.f;
            float height = 80.f;

            opt.bounds = sf::FloatRect({posX, posY - height/2.f}, {width * opt.scale, height * opt.scale});

            sf::RectangleShape rect({width, height});
            rect.setOrigin({0.f, height / 2.f});
            rect.setPosition({posX, posY});
            rect.setScale({opt.scale, opt.scale});
            
            bool isSelected = (i == m_selectedIndex);
            rect.setFillColor(isSelected ? sf::Color(0, 255, 255, 120) : sf::Color(40, 40, 60, 180));
            rect.setOutlineThickness(isSelected ? 4.f : 1.f);
            rect.setOutlineColor(isSelected ? sf::Color::White : sf::Color(100, 100, 100, 150));
            target.draw(rect);

            sf::Text text(*font, opt.label, static_cast<unsigned int>(40 * opt.scale));
            auto textBounds = text.getLocalBounds();
            text.setOrigin({0.f, textBounds.size.y / 2.f});
            text.setPosition({posX + 40.f * opt.scale, posY});
            text.setFillColor(isSelected ? sf::Color::White : sf::Color(200, 200, 200));
            target.draw(text);
        }
    }
}

} // namespace game::states
