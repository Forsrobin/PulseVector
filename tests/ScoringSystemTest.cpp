#include <gtest/gtest.h>
#include "game/systems/ScoringSystem.hpp"
#include "engine/core/EventBus.hpp"

TEST(ScoringSystemTest, PerfectHit) {
    engine::core::EventBus eventBus;
    game::systems::ScoringSystem scoringSystem(eventBus);
    
    eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Perfect, 0.0f, entt::null});
    
    EXPECT_EQ(scoringSystem.getScore(), 300);
    EXPECT_EQ(scoringSystem.getCombo(), 1);
}

TEST(ScoringSystemTest, ComboMultiplier) {
    engine::core::EventBus eventBus;
    game::systems::ScoringSystem scoringSystem(eventBus);
    
    // Get combo to 10
    for (int i = 0; i < 10; ++i) {
        eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Perfect, 0.0f, entt::null});
    }
    
    // At 10 combo, multiplier should be 2x
    // Score should be 10 * 300 * 1 = 3000 (wait, the multiplier is applied AFTER the increment in my code)
    // Actually in my code:
    // i=0: combo=1, mult=1, score=300
    // i=9: combo=10, mult=2, score=2700 + 300*2 = 3300 ?
    // Let's re-calculate:
    // 0-8: mult=1, score = 9 * 300 = 2700. combo goes from 1 to 9.
    // 9: combo=10, mult = 1 + (10/10) = 2. score = 2700 + 300*2 = 3300.
    
    EXPECT_EQ(scoringSystem.getScore(), 3300);
    EXPECT_EQ(scoringSystem.getCombo(), 10);
}

TEST(ScoringSystemTest, MissResetsCombo) {
    engine::core::EventBus eventBus;
    game::systems::ScoringSystem scoringSystem(eventBus);
    
    eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Perfect, 0.0f, entt::null});
    EXPECT_EQ(scoringSystem.getCombo(), 1);
    
    eventBus.publish(engine::core::HitEvent{engine::core::HitRating::Miss, 0.0f, entt::null});
    EXPECT_EQ(scoringSystem.getCombo(), 0);
}
