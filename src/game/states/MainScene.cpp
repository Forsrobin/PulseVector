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
#include "../components/HitObject.hpp"
#include "../components/Approach.hpp"
#include "../beatmap/BeatmapParser.hpp"
#include <fmt/core.h>

namespace game::states {

MainScene::MainScene(engine::core::Application& app, beatmap::Beatmap map)
    : m_app(app), m_beatmap(std::move(map)), m_particlePool(m_app.getRegistry()) {
}

void MainScene::onInitialize(entt::registry& registry) {
    fmt::print("Main Scene Initialized.\\n");

    // Task 8: Pre-allocate memory for common components during level load
    // Note: In EnTT 3.x, use .storage<T>().reserve(n)
    registry.storage<engine::graphics::components::Transform>().reserve(2000);
    registry.storage<engine::graphics::components::Particle>().reserve(1000);
    registry.storage<game::components::HitObject>().reserve(500);
    registry.storage<game::components::Approach>().reserve(500);

    // Add Systems
    addSystem(std::make_unique<engine::graphics::RenderSystem>(m_app.getRenderTarget()));
    addSystem(std::make_unique<engine::graphics::ParticleSystem>(m_app.getRenderTarget(), m_particlePool));
    
    // Gameplay Systems
    auto spriteMap = m_app.getTextureManager().get("sprite_map");
    auto defaultFont = m_app.getFontManager().get("default");

    addSystem(std::make_unique<systems::ScoringSystem>(m_app.getEventBus()));
    addSystem(std::make_unique<systems::HitSystem>(m_app.getAudioCore(), m_app.getEventBus()));
    addSystem(std::make_unique<systems::FeedbackSystem>(m_app.getEventBus(), m_app.getPostProcessManager(), m_particlePool, spriteMap, defaultFont));
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

    m_app.getEventBus().subscribe<engine::core::GameOverEvent>([this](const auto& event) {
        (void)event;
        this->m_isGameOver = true;
    });

    m_app.getEventBus().subscribe<engine::core::KeyEvent>([this](const auto& event) {
        if (event.key == sf::Keyboard::Key::R) {
            this->m_isRestarting = event.pressed;
            if (!event.pressed) {
                // If we were auto-restarting from game over, keep it.
                // But if it was a manual hold, reset timer if released early.
                if (!this->m_isGameOver) {
                    // Handled in update now
                }
            }
        }
    });

    m_backgroundDim.setSize({1280.f, 720.f});
    m_backgroundDim.setFillColor(sf::Color(0, 0, 0, 150));

    if (font) {
        m_restartText.emplace(*font, "RESTARTING");
        m_restartText->setCharacterSize(24);
        m_restartText->setFillColor(sf::Color::White);
    }

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

void MainScene::update(entt::registry& registry, sf::Time dt) {
    Scene::update(registry, dt);

    float elapsed = dt.asSeconds();

    // Hold R to Restart logic
    if (m_isRestarting || m_isGameOver) {
        m_restartTimer += elapsed;

        if (m_restartTimer >= 1.0f) {
            m_shouldRestart = true;
        }
    } else {
        // Decay if not pressing and not game over
        if (m_restartTimer > 0.f) {
            m_restartTimer -= elapsed * 2.0f;
            if (m_restartTimer < 0.f) m_restartTimer = 0.f;
        }
    }

    if (m_shouldRestart) {
        m_app.getAudioCore().stop();
        m_app.getAudioCore().setPlaybackSpeed(1.0f);
        m_app.getAudioCore().setVolume(100.f);
        m_app.getPostProcessManager().setBlurStrength(0.f);
        m_app.transitionToScene(std::make_unique<MainScene>(m_app, m_beatmap), sf::seconds(0.1f));
        return;
    }

    // Apply effects based on restart progress
    if (m_restartTimer > 0.f) {
        float progress = std::clamp(m_restartTimer, 0.f, 1.0f);
        m_app.getAudioCore().setPlaybackSpeed(1.0f - progress * 0.7f);
        m_app.getAudioCore().setVolume(100.f * (1.0f - progress * 0.8f));
        m_app.getPostProcessManager().setBlurStrength(progress * 5.0f);
    } else {
        m_app.getAudioCore().setPlaybackSpeed(1.0f);
        m_app.getAudioCore().setVolume(100.f);
        m_app.getPostProcessManager().setBlurStrength(0.f);
    }
}

void MainScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    float amplitude = m_app.getAudioCore().getAmplitude();
    float bass = m_app.getAudioCore().getBassEnergy();
    const auto& fft = m_app.getAudioCore().getFftData();

    // Update post-process with audio data for the background shader
    m_app.getPostProcessManager().updateAudioData(amplitude, bass, fft);
    
    // Much smoother background scaling: lower sensitivity and heavier smoothing
    float targetBass = bass * 0.6f; // Reduce raw sensitivity
    if (targetBass > m_smoothBass) {
        m_smoothBass += (targetBass - m_smoothBass) * 0.1f; // Fast but not instant pop
    } else {
        m_smoothBass -= (m_smoothBass - targetBass) * 0.02f; // Very slow decay
    }

    float bgScale = 1.0f + m_smoothBass * 0.08f; // Reduced max scale
    sf::View defaultView = target.getDefaultView();
    sf::View bgView = defaultView;
    bgView.zoom(1.0f / bgScale);
    target.setView(bgView);

    // Draw background grid with smoother pulsing and restart fade
    float restartAlphaMult = 1.0f - std::clamp(m_restartTimer, 0.f, 1.0f);
    
    // The background grid is now handled by the shader in PostProcessManager::begin()
    // We just need to manage the view transformations if needed, or move them to the shader too.
    // For now, let's keep the view zoom but remove the manual VertexArray drawing.

    target.setView(defaultView);

    // Central Reactive Circle (Visualizer)
    float baseRadius = 100.f;
    float reactiveRadius = baseRadius + m_smoothBass * 150.f;
    sf::CircleShape centralCircle(reactiveRadius);
    centralCircle.setOrigin({reactiveRadius, reactiveRadius});
    centralCircle.setPosition({640.f, 360.f});
    centralCircle.setOutlineThickness(4.f);
    centralCircle.setOutlineColor(sf::Color(0, 255, 255, static_cast<std::uint8_t>(180 * restartAlphaMult)));
    centralCircle.setFillColor(sf::Color(0, 255, 255, static_cast<std::uint8_t>(40 * restartAlphaMult)));
    target.draw(centralCircle);

    // Small inner pulse
    float innerRadius = 20.f + amplitude * 100.f;
    sf::CircleShape innerCircle(innerRadius);
    innerCircle.setOrigin({innerRadius, innerRadius});
    innerCircle.setPosition({640.f, 360.f});
    innerCircle.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(100 * restartAlphaMult)));
    target.draw(innerCircle);

    Scene::render(registry, interpolation);

    // Darken background during restart
    if (m_restartTimer > 0.f) {
        m_backgroundDim.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(m_restartTimer * 200.f)));
        target.draw(m_backgroundDim);
    }

    // UI Elements
    if (m_scoreText) {
        target.draw(*m_scoreText);
    }

    // Play Timeline (Gradient)
    float progress = 0.f;
    sf::Time current = m_app.getAudioCore().getSampleTime();
    sf::Time total = m_app.getAudioCore().getTotalDuration();
    if (total > sf::Time::Zero) {
        progress = current.asSeconds() / total.asSeconds();
    }

    float timelineHeight = 6.f;
    float timelineWidth = 1280.f * progress;
    
    sf::VertexArray timeline(sf::PrimitiveType::TriangleStrip, 4);
    sf::Color startColor = sf::Color(0, 255, 255, 200); // Cyan
    sf::Color endColor = sf::Color(255, 0, 255, 200);   // Magenta/Purple

    timeline[0].position = {0.f, 720.f - timelineHeight};
    timeline[0].color = startColor;
    timeline[1].position = {0.f, 720.f};
    timeline[1].color = startColor;
    
    // Interpolated color for the end of the bar
    sf::Color currentColor;
    currentColor.r = static_cast<std::uint8_t>(startColor.r + (endColor.r - startColor.r) * progress);
    currentColor.g = static_cast<std::uint8_t>(startColor.g + (endColor.g - startColor.g) * progress);
    currentColor.b = static_cast<std::uint8_t>(startColor.b + (endColor.b - startColor.b) * progress);
    currentColor.a = 200;

    timeline[2].position = {timelineWidth, 720.f - timelineHeight};
    timeline[2].color = currentColor;
    timeline[3].position = {timelineWidth, 720.f};
    timeline[3].color = currentColor;

    target.draw(timeline);

    // Restart UI overlay
    if (m_restartTimer > 0.f) {
        float progress = std::clamp(m_restartTimer, 0.f, 1.0f);
        sf::Vector2f center = {640.f, 360.f};
        
        // Progress circle outline
        float outerRadius = 80.f;
        sf::CircleShape ring(outerRadius);
        ring.setOrigin({outerRadius, outerRadius});
        ring.setPosition(center);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(5.f);
        ring.setOutlineColor(sf::Color(255, 255, 255, 100));
        target.draw(ring);

        // Circular progress (using VertexArray for a partial arc/circle)
        sf::VertexArray arc(sf::PrimitiveType::TriangleFan);
        arc.append(sf::Vertex(center, sf::Color(255, 0, 0, 180)));
        
        int points = 40;
        for (int i = 0; i <= points; ++i) {
            float angle = (static_cast<float>(i) / static_cast<float>(points)) * progress * 360.f - 90.f;
            float rad = angle * 3.14159f / 180.f;
            arc.append(sf::Vertex(center + sf::Vector2f(std::cos(rad) * outerRadius, std::sin(rad) * outerRadius), sf::Color(255, 0, 0, 180)));
        }
        target.draw(arc);

        if (m_restartText) {
            if (m_isGameOver) {
                m_restartText->setString("GAME OVER");
            } else {
                m_restartText->setString("RESTARTING");
            }
            auto bounds = m_restartText->getLocalBounds();
            m_restartText->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});
            m_restartText->setPosition(center);
            target.draw(*m_restartText);
        }
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
