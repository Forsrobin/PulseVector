#pragma once

#include "engine/core/Scene.hpp"
#include <SFML/Graphics/Text.hpp>
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
    std::optional<sf::Text> m_titleText;
    std::optional<sf::Text> m_promptText;
    float m_pulsateTimer{0.f};
};

} // namespace game::states
