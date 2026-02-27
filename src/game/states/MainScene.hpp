#pragma once

#include "engine/core/Scene.hpp"
#include "engine/audio/AudioCore.hpp"
#include "../beatmap/Beatmap.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "engine/core/Events.hpp"
#include "engine/utils/ObjectPool.hpp"
#include <SFML/Graphics/Text.hpp>
#include <optional>

namespace engine::core {
    class Application;
}

namespace game::states {

class MainScene : public engine::core::Scene {
public:
    MainScene(engine::core::Application& app, beatmap::Beatmap map);
    void onInitialize(entt::registry& registry) override;
    void update(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    void onScoreUpdate(const engine::core::ScoreUpdateEvent& event);

    engine::core::Application& m_app;
    beatmap::Beatmap m_beatmap;
    engine::utils::ObjectPool m_particlePool;

    std::optional<sf::Text> m_scoreText;
    int m_currentScore{0};
    int m_currentCombo{0};

    sf::RectangleShape m_timelineBar;
    sf::RectangleShape m_backgroundDim;
    std::optional<sf::Texture> m_levelBackgroundTexture;
    std::optional<sf::Sprite> m_levelBackgroundSprite;
    float m_smoothBass{0.f};
    float m_levelTimer{0.f};

    // Restart logic
    float m_restartTimer{0.f};
    bool m_isRestarting{false};
    bool m_shouldRestart{false};
    bool m_isGameOver{false};
    std::optional<sf::Text> m_restartText;
};

} // namespace game::states
