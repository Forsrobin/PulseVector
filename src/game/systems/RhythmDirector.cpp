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

    float currentAudioTime = m_audioCore.getPlaybackPosition().asSeconds();

    // Spawn nodes ahead of time
    while (m_nextNodeIndex < m_beatmap.nodes.size()) {
        const auto& node = m_beatmap.nodes[m_nextNodeIndex];
        float spawnTime = node.timeSeconds - m_approachTime;

        if (currentAudioTime >= spawnTime) {
            auto entity = registry.create();
            
            registry.emplace<engine::graphics::components::Transform>(entity, sf::Vector2f(node.x, node.y));
            
            registry.emplace<components::HitObject>(entity, node.timeSeconds, spawnTime, node.type, node.direction);

            // Add approach component for scaling animation
            registry.emplace<components::Approach>(entity, spawnTime, node.timeSeconds);

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
