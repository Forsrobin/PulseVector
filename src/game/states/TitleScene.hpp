#pragma once

#include "engine/core/Scene.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <optional>

namespace engine::core {
    class Application;
}

namespace game::states {

class TitleScene : public engine::core::Scene {
public:
    explicit TitleScene(engine::core::Application& app);
    void onInitialize(entt::registry& registry) override;
    void update(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    engine::core::Application& m_app;
    std::optional<sf::Sprite> m_backgroundSprite;
    std::optional<sf::Text> m_titleText;
    std::optional<sf::Text> m_promptText;
    float m_pulsateTimer{0.f};

    struct Decoration {
        sf::Vector2f position;
        float radius;
        sf::Color color;
        float speed;
        float phase;
    };
    std::vector<Decoration> m_decorations;
};

} // namespace game::states
