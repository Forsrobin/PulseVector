#pragma once

#include "engine/core/Scene.hpp"
#include "engine/core/AssetManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <vector>
#include <string>
#include <optional>

namespace engine::core {
    class Application;
}

namespace game::states {

struct MenuLevel {
    std::string title;
    std::string artist;
    std::string mapPath;
    sf::FloatRect bounds;
};

class MenuScene : public engine::core::Scene {
public:
    explicit MenuScene(engine::core::Application& app);
    void onInitialize(entt::registry& registry) override;
    void update(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    void loadLevel(const std::string& mapPath);

    engine::core::Application& m_app;
    std::optional<sf::Sprite> m_backgroundSprite;
    std::vector<MenuLevel> m_levels;
    std::optional<sf::Text> m_levelText;
    std::optional<sf::Sound> m_hoverSound;
    int m_selectedIndex{0};
    int m_hoveredIndex{-1};

    struct VisualState {
        float scale{1.0f};
        float offset{0.f};
    };
    std::vector<VisualState> m_visualStates;
};

} // namespace game::states
