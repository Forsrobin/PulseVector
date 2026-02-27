#pragma once

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <string>

#include "Timer.hpp"
#include "EventBus.hpp"
#include "InputSystem.hpp"
#include "AssetManager.hpp"
#include "Scene.hpp"
#include "../audio/AudioCore.hpp"
#include "../graphics/PostProcessManager.hpp"
#include <SFML/Audio/SoundBuffer.hpp>

namespace engine::core {

class Application {
public:
    Application(const std::string& title, uint32_t width, uint32_t height);
    virtual ~Application() = default;

    void run();
    void stop();

    void setScene(std::unique_ptr<Scene> scene);
    void transitionToScene(std::unique_ptr<Scene> scene, sf::Time duration = sf::seconds(0.5f));

    sf::RenderWindow& getWindow() { return m_window; }
    entt::registry& getRegistry() { return m_registry; }
    EventBus& getEventBus() { return m_eventBus; }
    InputSystem& getInputSystem() { return m_inputSystem; }
    audio::AudioCore& getAudioCore() { return m_audioCore; }
    graphics::PostProcessManager& getPostProcessManager() { return *m_postProcessManager; }

    AssetManager<sf::Texture>& getTextureManager() { return m_textureManager; }
    AssetManager<sf::Font>& getFontManager() { return m_fontManager; }
    AssetManager<sf::SoundBuffer>& getSoundManager() { return m_soundManager; }

    sf::RenderTarget& getRenderTarget();

protected:
    virtual void onInitialize() = 0;
    virtual void onUpdate(sf::Time dt) = 0;
    virtual void onFixedUpdate(sf::Time dt) = 0;
    virtual void onRender(float interpolation) = 0;
    virtual void onShutdown() = 0;

private:
    void processEvents();
    void updateTransition(sf::Time dt);
    void renderTransition();

    sf::RenderWindow m_window;
    entt::registry m_registry;
    EventBus m_eventBus;
    InputSystem m_inputSystem;
    audio::AudioCore m_audioCore;
    Timer m_timer;
    bool m_running;

    AssetManager<sf::Texture> m_textureManager;
    AssetManager<sf::Font> m_fontManager;
    AssetManager<sf::SoundBuffer> m_soundManager;

    std::unique_ptr<graphics::PostProcessManager> m_postProcessManager;
    std::unique_ptr<Scene> m_currentScene;

    // Transition state
    std::unique_ptr<Scene> m_nextScene;
    sf::Time m_transitionTimer{sf::Time::Zero};
    sf::Time m_transitionDuration{sf::Time::Zero};
    bool m_isTransitioning{false};
};

} // namespace engine::core
