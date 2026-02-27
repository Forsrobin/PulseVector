#include "PulseVectorGame.hpp"
#include "states/MainScene.hpp"
#include "states/MenuScene.hpp"
#include "states/TitleScene.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <fmt/core.h>

namespace game {

PulseVectorGame::PulseVectorGame()
    : engine::core::Application("Pulse Vector", 1280, 720) {
}

void PulseVectorGame::onInitialize() {
    fmt::print("Pulse Vector Game Initialized.\n");

    // Preload basic assets
    getTextureManager().load("sprite_map", "assets/textures/sprite_map.png");
    getTextureManager().load("background_1", "assets/textures/background_1.png");
    getTextureManager().load("background_2", "assets/textures/background_2.png");
    getFontManager().load("default", "assets/fonts/Jersey25-Regular.ttf");
    getSoundManager().load("hover", "assets/audio/hover.mp3");

    // Start with TitleScene
    setScene(std::make_unique<states::TitleScene>(*this));
}

void PulseVectorGame::onUpdate(sf::Time dt) {
    (void)dt;
}

void PulseVectorGame::onFixedUpdate(sf::Time dt) {
    (void)dt;
}

void PulseVectorGame::onRender(float interpolation) {
    (void)interpolation;
}

void PulseVectorGame::onShutdown() {
    fmt::print("Pulse Vector Game Shutting Down.\n");
}

} // namespace game
