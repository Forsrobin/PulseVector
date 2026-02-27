#pragma once

#include "engine/ecs/System.hpp"
#include "engine/audio/AudioCore.hpp"
#include "../beatmap/Beatmap.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <memory>

namespace game::systems {

class RhythmDirector : public engine::ecs::ISystem {
public:
    RhythmDirector(engine::audio::AudioCore& audioCore, beatmap::Beatmap map, std::shared_ptr<sf::Texture> spriteMap);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    engine::audio::AudioCore& m_audioCore;
    beatmap::Beatmap m_beatmap;
    std::shared_ptr<sf::Texture> m_spriteMap;
    
    float m_approachTime{1.5f}; // Seconds before node time to spawn entity
    size_t m_nextNodeIndex{0};
};

} // namespace game::systems
