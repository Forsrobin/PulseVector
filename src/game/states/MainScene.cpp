#include "MainScene.hpp"
#include "engine/core/Application.hpp"
#include "engine/graphics/RenderSystem.hpp"
#include "engine/graphics/ParticleSystem.hpp"
#include "engine/graphics/components/Transform.hpp"
#include "engine/graphics/components/Particle.hpp"
#include "../systems/RhythmDirector.hpp"
#include "../systems/HitSystem.hpp"
#include "../systems/ScoringSystem.hpp"
#include "../systems/FeedbackSystem.hpp"
#include "../systems/ApproachSystem.hpp"
#include "../beatmap/BeatmapParser.hpp"
#include <fmt/core.h>

namespace game::states {

MainScene::MainScene(engine::core::Application& app, beatmap::Beatmap map)
    : m_app(app), m_beatmap(std::move(map)) {
}

void MainScene::onInitialize(entt::registry& registry) {
    fmt::print("Main Scene Initialized.\\n");

    // Add Systems
    addSystem(std::make_unique<engine::graphics::RenderSystem>(m_app.getRenderTarget()));
    addSystem(std::make_unique<engine::graphics::ParticleSystem>(m_app.getRenderTarget()));
    
    // Gameplay Systems
    auto spriteMap = m_app.getTextureManager().get("sprite_map");
    auto defaultFont = m_app.getFontManager().get("default");

    addSystem(std::make_unique<systems::ScoringSystem>(m_app.getEventBus()));
    addSystem(std::make_unique<systems::HitSystem>(m_app.getAudioCore(), m_app.getEventBus()));
    addSystem(std::make_unique<systems::FeedbackSystem>(m_app.getEventBus(), m_app.getPostProcessManager(), spriteMap, defaultFont));
    addSystem(std::make_unique<systems::ApproachSystem>(m_app.getAudioCore()));
    
    // Rhythm Director with the actual beatmap and sprite map
    addSystem(std::make_unique<systems::RhythmDirector>(m_app.getAudioCore(), m_beatmap, spriteMap));

    // UI Setup
    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_scoreText.emplace(*font, "Score: 0\nCombo: 0");
        m_scoreText->setCharacterSize(30);
        m_scoreText->setFillColor(sf::Color::White);
        m_scoreText->setPosition({20.f, 20.f});
    }

    m_app.getEventBus().subscribe<engine::core::ScoreUpdateEvent>([this](const auto& event) {
        this->onScoreUpdate(event);
    });

    // Load and play the music
    if (m_app.getAudioCore().loadMusic(m_beatmap.audioPath)) {
        m_app.getAudioCore().play();
    } else {
        fmt::print(stderr, "Failed to load music for level: {}\\n", m_beatmap.audioPath);
    }

    // Create a burst of particles
    for (int i = 0; i < 100; ++i) {
        auto entity = registry.create();
        registry.emplace<engine::graphics::components::Transform>(entity, sf::Vector2f(640.f, 360.f));
        
        float angle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 360.f;
        float speed = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 200.f + 50.f;
        sf::Vector2f velocity(cos(angle) * speed, sin(angle) * speed);

        registry.emplace<engine::graphics::components::Particle>(entity, 
            velocity, 
            0.f, 
            sf::seconds(1.f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))),
            sf::Time::Zero,
            sf::Color::Cyan,
            sf::Color::Blue,
            5.f,
            0.f
        );
    }
}

void MainScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    float amplitude = m_app.getAudioCore().getAmplitude();
    
    // Draw background grid with pulsing effect
    sf::VertexArray grid(sf::PrimitiveType::Lines);
    // Pulse grid color based on amplitude
    std::uint8_t pulseVal = static_cast<std::uint8_t>(40 + amplitude * 100.f);
    sf::Color gridColor(pulseVal, pulseVal, pulseVal + 20);
    
    float spacing = 80.f + amplitude * 20.f; // Grid lines move slightly with beat
    float offset = (std::sin(m_app.getAudioCore().getSmoothedPosition().asSeconds() * 2.f) * 10.f);

    for (float x = offset; x <= 1280 + spacing; x += spacing) {
        grid.append(sf::Vertex({x, 0}, gridColor));
        grid.append(sf::Vertex({x, 720}, gridColor));
    }
    for (float y = offset; y <= 720 + spacing; y += spacing) {
        grid.append(sf::Vertex({0, y}, gridColor));
        grid.append(sf::Vertex({1280, y}, gridColor));
    }
    target.draw(grid);

    // Audio Visualizer Bars
    const auto& fft = m_app.getAudioCore().getFftData();
    float barWidth = 1280.f / static_cast<float>(fft.size());
    for (size_t i = 0; i < fft.size(); ++i) {
        float barHeight = fft[i] * 800.f;
        sf::RectangleShape bar({barWidth - 2.f, barHeight});
        bar.setPosition({static_cast<float>(i) * barWidth, 720.f - barHeight});
        bar.setFillColor(sf::Color(0, 255, 255, 100));
        target.draw(bar);
    }

    Scene::render(registry, interpolation);

    if (m_scoreText) {
        target.draw(*m_scoreText);
    }
}

void MainScene::onScoreUpdate(const engine::core::ScoreUpdateEvent& event) {
    m_currentScore = event.currentScore;
    m_currentCombo = event.currentCombo;
    if (m_scoreText) {
        m_scoreText->setString(fmt::format("Score: {}\nCombo: {}", m_currentScore, m_currentCombo));
    }
}

} // namespace game::states
