#include "Application.hpp"
#include <fmt/core.h>

namespace engine::core {

Application::Application(const std::string& title, uint32_t width, uint32_t height)
    : m_window(sf::VideoMode({width, height}), title)
    , m_inputSystem(m_eventBus)
    , m_timer(sf::milliseconds(4)) // Fixed timestep at 240Hz
    , m_running(false) {
    m_window.setFramerateLimit(0); // VSync or uncapped
    m_postProcessManager = std::make_unique<graphics::PostProcessManager>(width, height);
}

void Application::run() {
    m_running = true;
    onInitialize();

    while (m_running && m_window.isOpen()) {
        m_timer.update();
        processEvents();
        m_audioCore.update();
        m_postProcessManager->update(m_timer.getDeltaTime());
        updateTransition(m_timer.getDeltaTime());

        onUpdate(m_timer.getDeltaTime());
        if (m_currentScene) {
            m_currentScene->update(m_registry, m_timer.getDeltaTime());
        }

        while (m_timer.shouldUpdateFixed()) {
            onFixedUpdate(m_timer.getFixedTimeStep());
            if (m_currentScene) {
                m_currentScene->fixedUpdate(m_registry, m_timer.getFixedTimeStep());
            }
            m_timer.consumeFixedUpdate();
        }

        m_window.clear(sf::Color::Black);
        
        onRender(m_timer.getInterpolationFactor());
        if (m_currentScene) {
            m_currentScene->render(m_registry, m_timer.getInterpolationFactor());
        }

        renderTransition();

        m_window.display();
    }

    onShutdown();
}

sf::RenderTarget& Application::getRenderTarget() {
    return m_window;
}

void Application::stop() {
    m_running = false;
}

void Application::setScene(std::unique_ptr<Scene> scene) {
    if (m_currentScene) {
        m_currentScene->onShutdown(m_registry);
    }
    m_currentScene = std::move(scene);
    if (m_currentScene) {
        m_currentScene->onInitialize(m_registry);
    }
}

void Application::transitionToScene(std::unique_ptr<Scene> scene, sf::Time duration) {
    if (m_isTransitioning) return;
    m_nextScene = std::move(scene);
    m_transitionDuration = duration;
    m_transitionTimer = sf::Time::Zero;
    m_isTransitioning = true;
}

void Application::updateTransition(sf::Time dt) {
    if (!m_isTransitioning) return;

    m_transitionTimer += dt;
    if (m_transitionTimer >= m_transitionDuration / 2.f && m_nextScene) {
        setScene(std::move(m_nextScene));
    }

    if (m_transitionTimer >= m_transitionDuration) {
        m_isTransitioning = false;
    }
}

void Application::renderTransition() {
    if (!m_isTransitioning) return;

    float halfDuration = m_transitionDuration.asSeconds() / 2.f;
    float elapsed = m_transitionTimer.asSeconds();
    float alpha = 0.f;

    if (elapsed < halfDuration) {
        alpha = elapsed / halfDuration;
    } else {
        alpha = 1.f - ((elapsed - halfDuration) / halfDuration);
    }

    sf::RectangleShape fadeRect(sf::Vector2f(m_window.getSize()));
    fadeRect.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha * 255.f)));
    m_window.draw(fadeRect);
}

void Application::processEvents() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            m_running = false;
        } else {
            m_inputSystem.processEvent(*event, m_window);
        }
    }
}

} // namespace engine::core
