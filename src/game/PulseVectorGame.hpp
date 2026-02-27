#pragma once

#include "engine/core/Application.hpp"

namespace game {

class PulseVectorGame : public engine::core::Application {
public:
    PulseVectorGame();

protected:
    void onInitialize() override;
    void onUpdate(sf::Time dt) override;
    void onFixedUpdate(sf::Time dt) override;
    void onRender(float interpolation) override;
    void onShutdown() override;

private:
    // Game-specific members here
};

} // namespace game
