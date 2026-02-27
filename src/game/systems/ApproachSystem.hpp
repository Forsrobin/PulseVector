#pragma once

#include "engine/ecs/System.hpp"
#include "engine/audio/AudioCore.hpp"

namespace game::systems {

class ApproachSystem : public engine::ecs::ISystem {
public:
    explicit ApproachSystem(engine::audio::AudioCore& audioCore);

    void update(entt::registry& registry, sf::Time dt) override;
    void fixedUpdate(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    engine::audio::AudioCore& m_audioCore;
};

} // namespace game::systems
