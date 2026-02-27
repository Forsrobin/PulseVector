#include "RhythmDirector.hpp"
#include "../components/HitObject.hpp"
#include "../components/Approach.hpp"
#include "engine/graphics/components/Transform.hpp"
#include "engine/graphics/components/Renderable.hpp"
#include <algorithm>

namespace game::systems {

RhythmDirector::RhythmDirector(engine::audio::AudioCore& audioCore, beatmap::Beatmap map, std::shared_ptr<sf::Texture> spriteMap)
    : m_audioCore(audioCore), m_beatmap(std::move(map)), m_spriteMap(std::move(spriteMap)) {
    // Sort nodes by time to ensure sequential spawning
    std::ranges::sort(m_beatmap.nodes, [](const auto& a, const auto& b) {
        return a.timeSeconds < b.timeSeconds;
    });
}

void RhythmDirector::update(entt::registry& registry, sf::Time dt) {
    (void)dt;

    if (!m_audioCore.isPlaying()) {
        return;
    }

    float currentAudioTime = m_audioCore.getSampleTime().asSeconds();

    // Spawn nodes ahead of time
    while (m_nextNodeIndex < m_beatmap.nodes.size()) {
        const auto& node = m_beatmap.nodes[m_nextNodeIndex];
        float spawnTime = node.timeSeconds - m_approachTime;

        if (currentAudioTime >= spawnTime) {
            auto entity = registry.create();
            
            registry.emplace<engine::graphics::components::Transform>(entity, sf::Vector2f(node.x, node.y));
            
            registry.emplace<components::HitObject>(entity, node.timeSeconds, spawnTime, node.type, node.direction);

            // Add approach component for scaling and spline animation
            auto& approach = registry.emplace<components::Approach>(entity, spawnTime, node.timeSeconds);
            
            // Generate path points
            if (!node.curvePoints.empty()) {
                for (const auto& p : node.curvePoints) {
                    approach.pathPoints.push_back({p.first, p.second});
                }
            } else {
                // Default path: Spawn from off-screen towards target
                sf::Vector2f targetPos(node.x, node.y);
                sf::Vector2f spawnPos = targetPos;
                
                // Offset spawn position based on direction
                float offset = 1000.f;
                switch (node.direction) {
                    case 0: spawnPos.y -= offset; break; // Up -> comes from top
                    case 1: spawnPos.x += offset; break; // Right -> comes from right
                    case 2: spawnPos.y += offset; break; // Down -> comes from bottom
                    case 3: spawnPos.x -= offset; break; // Left -> comes from left
                }

                // Simple 3-point spline for some curve feel even on defaults
                approach.pathPoints.push_back(spawnPos);
                // Mid point with slight offset for "swing"
                sf::Vector2f mid = (spawnPos + targetPos) * 0.5f;
                if (node.direction == 0 || node.direction == 2) mid.x += 100.f;
                else mid.y += 100.f;
                
                approach.pathPoints.push_back(mid);
                approach.pathPoints.push_back(targetPos);
            }

            // Visual representation using the sprite map
            if (m_spriteMap) {
                engine::graphics::components::Renderable renderable;
                renderable.texture = m_spriteMap;
                // (0,0) Up, (0,1) Right, (0,2) Down, (0,3) Left
                // User said: (0,0) => up, (0,1) => right... if Row 0, Col X:
                renderable.textureRect = sf::IntRect({node.direction * 32, 0}, {32, 32});
                registry.emplace<engine::graphics::components::Renderable>(entity, renderable);
            }

            m_nextNodeIndex++;
        } else {
            break;
        }
    }
}

void RhythmDirector::fixedUpdate(entt::registry& registry, sf::Time dt) {
    (void)registry;
    (void)dt;
}

void RhythmDirector::render(entt::registry& registry, float interpolation) {
    (void)registry;
    (void)interpolation;
}

} // namespace game::systems
