#include <gtest/gtest.h>
#include "game/systems/FeedbackSystem.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/graphics/components/LifeTime.hpp"
#include "engine/graphics/PostProcessManager.hpp"
#include <entt/entt.hpp>

TEST(FeedbackSystemTest, SpawnsOnHit) {
    engine::core::EventBus eventBus;
    entt::registry registry;
    engine::graphics::PostProcessManager postProcess(1280, 720);
    
    game::systems::FeedbackSystem feedbackSystem(eventBus, postProcess, nullptr, nullptr);
    
    // We need to call update once to set the registry pointer in my current implementation
    feedbackSystem.update(registry, sf::Time::Zero);
    
    eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Perfect, 0.0f, entt::null, {100.f, 100.f}});
    
    auto view = registry.view<engine::graphics::components::LifeTime>();
    EXPECT_EQ(view.size(), 1);
}

TEST(FeedbackSystemTest, CleanupOnExpiredLife) {
    engine::core::EventBus eventBus;
    entt::registry registry;
    engine::graphics::PostProcessManager postProcess(1280, 720);
    
    game::systems::FeedbackSystem feedbackSystem(eventBus, postProcess, nullptr, nullptr);
    feedbackSystem.update(registry, sf::Time::Zero);
    
    eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Perfect, 0.0f, entt::null, {100.f, 100.f}});
    
    EXPECT_EQ(registry.view<engine::graphics::components::LifeTime>().size(), 1);
    
    feedbackSystem.update(registry, sf::seconds(1.0f));
    
    EXPECT_EQ(registry.view<engine::graphics::components::LifeTime>().size(), 0);
}
