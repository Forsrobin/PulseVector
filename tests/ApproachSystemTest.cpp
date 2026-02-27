#include <gtest/gtest.h>
#include "game/systems/ApproachSystem.hpp"
#include "game/components/Approach.hpp"
#include "engine/graphics/components/Transform.hpp"
#include "engine/audio/AudioCore.hpp"

// We can't easily mock AudioCore without interfaces, but we can test if it compiles and runs basic logic
// In a real project we'd use a MockAudioCore or similar.

TEST(ApproachSystemTest, ScalingLogic) {
    entt::registry registry;
    engine::audio::AudioCore audioCore; // Will not be playing
    
    game::systems::ApproachSystem system(audioCore);
    
    auto entity = registry.create();
    registry.emplace<engine::graphics::components::Transform>(entity);
    registry.emplace<game::components::Approach>(entity, 0.0f, 1.0f, 4.0f, 1.0f);
    
    // If not playing, scale should stay default (1,1)
    system.update(registry, sf::Time::Zero);
    auto& transform = registry.get<engine::graphics::components::Transform>(entity);
    EXPECT_FLOAT_EQ(transform.scale.x, 1.0f);
}
