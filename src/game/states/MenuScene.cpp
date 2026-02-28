#include "MenuScene.hpp"
#include "MainScene.hpp"
#include "TitleScene.hpp"
#include "SettingsScene.hpp"
#include "LevelEditorMenuScene.hpp"
#include "engine/core/Application.hpp"
#include "engine/graphics/RenderSystem.hpp"
#include "../beatmap/BeatmapParser.hpp"
#include <fmt/core.h>

namespace game::states {

MenuScene::MenuScene(engine::core::Application& app)
    : m_app(app) {
}

void MenuScene::onInitialize(entt::registry& registry) {
    // Load background
    auto bgTex = m_app.getTextureManager().get("background_2");
    if (bgTex) {
        m_backgroundSprite.emplace(*bgTex);
        sf::Vector2u texSize = bgTex->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            m_backgroundSprite->setScale({1280.f / texSize.x, 720.f / texSize.y});
        }
        m_backgroundSprite->setColor(sf::Color(255, 255, 255, 120)); // Dim for menu readability
    }

    // Load font
    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_levelText.emplace(*font, "");
        m_levelText->setCharacterSize(36);
        m_levelText->setFillColor(sf::Color::White);
    }

    // Load hover sound
    auto soundBuffer = m_app.getSoundManager().get("hover");
    if (soundBuffer) {
        m_hoverSound.emplace(*soundBuffer);
    }

    // Define dummy levels for testing
    m_levels.clear();
    m_levels.push_back({"Hentai 2017", "S3RL", "levels/S3RL-Hentai_2017.pvmap", {}});
    m_levels.push_back({"MOCK SONG 1", "Artist A", "levels/nonexistent1.pvmap", {}});
    m_levels.push_back({"LEVEL EDITOR", "Create & Edit", "LEVEL_EDITOR", {}});
    m_levels.push_back({"SETTINGS", "Configure", "SETTINGS", {}});

    m_visualStates.resize(m_levels.size());

    if (font) {
        m_homeText.emplace(*font, "Home");
        m_homeText->setCharacterSize(40);
        m_homeText->setFillColor(sf::Color::White);
        auto homeBounds = m_homeText->getLocalBounds();
        m_homeText->setOrigin({0.f, homeBounds.size.y / 2.f});
        m_homeText->setPosition({50.f, 670.f});
        m_homeBounds = sf::FloatRect({50.f, 670.f - homeBounds.size.y/2.f}, homeBounds.size);
    }
}

void MenuScene::update(entt::registry& registry, sf::Time dt) {
    Scene::update(registry, dt);

    float elapsed = dt.asSeconds();

    // Keyboard Navigation
    static bool upPressed = false;
    static bool downPressed = false;
    static bool enterPressed = false;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        if (!upPressed) {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_levels.size())) % m_levels.size();
            if (m_hoverSound) m_hoverSound->play();
            upPressed = true;
        }
    } else {
        upPressed = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        if (!downPressed) {
            m_selectedIndex = (m_selectedIndex + 1) % m_levels.size();
            if (m_hoverSound) m_hoverSound->play();
            downPressed = true;
        }
    } else {
        downPressed = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter)) {
        if (!enterPressed) {
            loadLevel(m_levels[m_selectedIndex].mapPath);
            enterPressed = true;
        }
    } else {
        enterPressed = false;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
        m_app.transitionToScene(std::make_unique<TitleScene>(m_app));
    }

    // Mouse Interaction
    auto mousePos = m_app.getInputSystem().getMousePosition();
    
    // Home button check
    if (m_homeBounds.contains(mousePos)) {
        if (m_homeText && m_homeText->getFillColor() != sf::Color::Cyan) {
            if (m_hoverSound) m_hoverSound->play();
            m_homeText->setFillColor(sf::Color::Cyan);
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            m_app.transitionToScene(std::make_unique<TitleScene>(m_app));
        }
    } else {
        if (m_homeText) m_homeText->setFillColor(sf::Color::White);
    }

    for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
        if (m_levels[i].bounds.contains(mousePos)) {
            if (m_selectedIndex != i) {
                m_selectedIndex = i;
                if (m_hoverSound) m_hoverSound->play();
            }
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                loadLevel(m_levels[i].mapPath);
            }
            break;
        }
    }

    // Smooth Visual Interpolation
    for (int i = 0; i < static_cast<int>(m_visualStates.size()); ++i) {
        float targetScale = (i == m_selectedIndex) ? 1.15f : 1.0f;
        float targetOffset = (i == m_selectedIndex) ? 40.f : 0.f;

        m_visualStates[i].scale += (targetScale - m_visualStates[i].scale) * 15.f * elapsed;
        m_visualStates[i].offset += (targetOffset - m_visualStates[i].offset) * 15.f * elapsed;
    }
}

void MenuScene::loadLevel(const std::string& mapPath) {
    if (mapPath == "SETTINGS") {
        m_app.transitionToScene(std::make_unique<SettingsScene>(m_app));
        return;
    }

    if (mapPath == "LEVEL_EDITOR") {
        m_app.transitionToScene(std::make_unique<LevelEditorMenuScene>(m_app));
        return;
    }

    auto map = game::beatmap::BeatmapParser::parse(mapPath);
    if (map) {
        if (map->packagePath.empty()) {
            // Correct the audio path relative to the map
            std::string dir = "";
            auto lastSlash = mapPath.find_last_of("/\\\\");
            if (lastSlash != std::string::npos) {
                dir = mapPath.substr(0, lastSlash + 1);
            }
            map->audioPath = dir + map->audioPath;
        }

        m_app.transitionToScene(std::make_unique<MainScene>(m_app, std::move(*map)), sf::seconds(1.0f));
    } else {
        fmt::print(stderr, "Failed to load map: {}\n", mapPath);
    }
}

void MenuScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    if (m_backgroundSprite) {
        target.draw(*m_backgroundSprite);
    }

    Scene::render(registry, interpolation);
    auto font = m_app.getFontManager().get("default");

    if (font) {
        sf::Text title(*font, "SONG SELECT", 70);
        title.setFillColor(sf::Color::Cyan);
        auto titleBounds = title.getLocalBounds();
        title.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
        title.setPosition({640.f, 80.f});
        target.draw(title);
    }

    if (!m_levelText) return;

    float startX = 200.f;
    float startY = 220.f;
    float spacingY = 85.f;
    float diagonalStepX = 30.f;

    for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
        auto& level = m_levels[i];
        const auto& visual = m_visualStates[i];

        float posX = startX + (i * diagonalStepX) + visual.offset;
        float posY = startY + (i * spacingY);
        float width = 600.f;
        float height = 70.f;

        // Update bounds for mouse interaction
        level.bounds = sf::FloatRect({posX, posY}, {width * visual.scale, height * visual.scale});

        sf::RectangleShape rect({width, height});
        rect.setOrigin({0.f, height / 2.f});
        rect.setPosition({posX, posY});
        rect.setScale({visual.scale, visual.scale});
        
        bool isSelected = (i == m_selectedIndex);
        rect.setFillColor(isSelected ? sf::Color(0, 255, 255, 100) : sf::Color(40, 40, 60, 180));
        rect.setOutlineThickness(isSelected ? 3.f : 1.f);
        rect.setOutlineColor(isSelected ? sf::Color::White : sf::Color(100, 100, 100, 150));
        
        target.draw(rect);

        m_levelText->setString(fmt::format("{} - {}", level.artist, level.title));
        m_levelText->setCharacterSize(static_cast<unsigned int>(32 * visual.scale));
        auto textBounds = m_levelText->getLocalBounds();
        m_levelText->setOrigin({0.f, textBounds.size.y / 2.f});
        m_levelText->setPosition({posX + 30.f * visual.scale, posY});
        
        if (isSelected) {
            m_levelText->setFillColor(sf::Color::White);
        } else {
            m_levelText->setFillColor(sf::Color(180, 180, 180));
        }
        
        target.draw(*m_levelText);
    }

    if (m_homeText) {
        target.draw(*m_homeText);
    }
}

} // namespace game::states
