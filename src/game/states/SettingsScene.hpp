#pragma once

#include "engine/core/Scene.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <optional>

namespace engine::core {
    class Application;
}

namespace game::states {

class SettingsScene : public engine::core::Scene {
public:
    explicit SettingsScene(engine::core::Application& app);
    void onInitialize(entt::registry& registry) override;
    void update(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    engine::core::Application& m_app;
    std::optional<sf::Sprite> m_backgroundSprite;
    std::optional<sf::Text> m_titleText;
    std::optional<sf::Text> m_volumeText;
    std::optional<sf::Text> m_backText;
    std::optional<sf::Text> m_homeText;

    sf::RectangleShape m_sliderBar;
    sf::RectangleShape m_sliderKnob;
    sf::FloatRect m_homeBounds;
    
    float m_volume;
    bool m_isDragging{false};
};

} // namespace game::states
