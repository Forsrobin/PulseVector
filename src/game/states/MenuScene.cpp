#include "MenuScene.hpp"
#include "MainScene.hpp"
#include "engine/core/Application.hpp"
#include "engine/graphics/RenderSystem.hpp"
#include "../beatmap/BeatmapParser.hpp"
#include <fmt/core.h>

namespace game::states {

MenuScene::MenuScene(engine::core::Application& app)
    : m_app(app) {
}

void MenuScene::onInitialize(entt::registry& registry) {
    fmt::print("Menu Scene Initialized.\\n");
    addSystem(std::make_unique<engine::graphics::RenderSystem>(m_app.getRenderTarget()));

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
    m_levels.push_back({"Hentai 2017", "S3RL", "levels/S3RL-Hentai_2017.pvmap", sf::FloatRect({400.f, 200.f}, {480.f, 60.f})});
    // Add some more mock levels to fill the menu
    m_levels.push_back({"MOCK SONG 1", "Artist A", "levels/nonexistent1.pvmap", sf::FloatRect({400.f, 280.f}, {480.f, 60.f})});
    m_levels.push_back({"MOCK SONG 2", "Artist B", "levels/nonexistent2.pvmap", sf::FloatRect({400.f, 360.f}, {480.f, 60.f})});
    m_levels.push_back({"MOCK SONG 3", "Artist C", "levels/nonexistent3.pvmap", sf::FloatRect({400.f, 440.f}, {480.f, 60.f})});
}

void MenuScene::update(entt::registry& registry, sf::Time dt) {
    Scene::update(registry, dt);

    auto mousePos = m_app.getInputSystem().getMousePosition();
    int newHoverIndex = -1;

    for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
        if (m_levels[i].bounds.contains(mousePos)) {
            newHoverIndex = i;
            break;
        }
    }

    if (newHoverIndex != m_hoveredIndex) {
        if (newHoverIndex != -1 && m_hoverSound) {
            m_hoverSound->setPitch(1.0f + (static_cast<float>(newHoverIndex) * 0.1f));
            m_hoverSound->play();
        }
        m_hoveredIndex = newHoverIndex;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && m_hoveredIndex != -1) {
        loadLevel(m_levels[m_hoveredIndex].mapPath);
    }
}

void MenuScene::loadLevel(const std::string& mapPath) {
    auto map = game::beatmap::BeatmapParser::parse(mapPath);
    if (map) {
        // Correct the audio path relative to the map
        std::string dir = "";
        auto lastSlash = mapPath.find_last_of("/\\\\");
        if (lastSlash != std::string::npos) {
            dir = mapPath.substr(0, lastSlash + 1);
        }
        map->audioPath = dir + map->audioPath;

        m_app.transitionToScene(std::make_unique<MainScene>(m_app, std::move(*map)), sf::seconds(1.0f));
    } else {
        fmt::print(stderr, "Failed to load map: {}\\n", mapPath);
    }
}

void MenuScene::render(entt::registry& registry, float interpolation) {
    Scene::render(registry, interpolation);

    auto& target = m_app.getRenderTarget();
    auto font = m_app.getFontManager().get("default");

    if (font) {
        sf::Text title(*font, "SELECT SONG", 60);
        title.setFillColor(sf::Color::Yellow);
        auto titleBounds = title.getLocalBounds();
        title.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
        title.setPosition({640.f, 100.f});
        target.draw(title);
    }

    if (!m_levelText) return;

    for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
        const auto& level = m_levels[i];
        
        // Background for item
        sf::RectangleShape rect(level.bounds.size);
        rect.setPosition(level.bounds.position);
        rect.setFillColor(i == m_hoveredIndex ? sf::Color(60, 60, 100) : sf::Color(40, 40, 60));
        rect.setOutlineThickness(2.f);
        rect.setOutlineColor(i == m_hoveredIndex ? sf::Color::Cyan : sf::Color(100, 100, 100));
        target.draw(rect);

        m_levelText->setString(fmt::format("{} - {}", level.artist, level.title));
        auto textBounds = m_levelText->getLocalBounds();
        m_levelText->setOrigin({0.f, textBounds.size.y / 2.f});
        m_levelText->setPosition({level.bounds.position.x + 20.f, level.bounds.position.y + level.bounds.size.y / 2.f});
        
        if (i == m_hoveredIndex) {
            m_levelText->setFillColor(sf::Color::White);
        } else {
            m_levelText->setFillColor(sf::Color(200, 200, 200));
        }
        
        target.draw(*m_levelText);
    }
}

} // namespace game::states
