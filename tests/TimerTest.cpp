#include <gtest/gtest.h>
#include "engine/core/Timer.hpp"
#include <SFML/System/Sleep.hpp>

TEST(TimerTest, DeltaTime) {
    engine::core::Timer timer;
    sf::sleep(sf::milliseconds(10));
    timer.update();
    
    EXPECT_GT(timer.getDeltaTime().asMilliseconds(), 0);
    EXPECT_LT(timer.getDeltaTime().asMilliseconds(), 50); // Reasonable upper bound
}

TEST(TimerTest, FixedUpdate) {
    engine::core::Timer timer(sf::milliseconds(10));
    sf::sleep(sf::milliseconds(25));
    timer.update();
    
    int count = 0;
    while (timer.shouldUpdateFixed()) {
        count++;
        timer.consumeFixedUpdate();
    }
    
    EXPECT_EQ(count, 2);
}

TEST(TimerTest, Interpolation) {
    engine::core::Timer timer(sf::milliseconds(10));
    sf::sleep(sf::milliseconds(15));
    timer.update();
    
    EXPECT_TRUE(timer.shouldUpdateFixed());
    timer.consumeFixedUpdate();
    
    EXPECT_FALSE(timer.shouldUpdateFixed());
    EXPECT_GT(timer.getInterpolationFactor(), 0.0f);
    EXPECT_LT(timer.getInterpolationFactor(), 1.0f);
}
