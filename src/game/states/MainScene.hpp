#pragma once

#include "engine/core/Scene.hpp"
#include "engine/audio/AudioCore.hpp"
#include "../beatmap/Beatmap.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

#include "engine/core/Events.hpp"
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
    void render(entt::registry& registry, float interpolation) override;

private:
    void onScoreUpdate(const engine::core::ScoreUpdateEvent& event);

    engine::core::Application& m_app;
    beatmap::Beatmap m_beatmap;

    std::optional<sf::Text> m_scoreText;
    int m_currentScore{0};
    int m_currentCombo{0};
};

} // namespace game::states
