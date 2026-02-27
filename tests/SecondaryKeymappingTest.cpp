#include <gtest/gtest.h>
#include "game/systems/HitSystem.hpp"
#include "engine/core/EventBus.hpp"
#include "engine/audio/AudioCore.hpp"

// We add a friend-like access or just check the internal mapping if possible
// Since m_keyToDirection is private, we can't check it directly without changes.
// However, we can test the behavior by publishing events.

TEST(HitSystemTest, SecondaryKeyMapping) {
    engine::core::EventBus eventBus;
    engine::audio::AudioCore audioCore;
    game::systems::HitSystem hitSystem(audioCore, eventBus);

    // Verify Arrow Keys
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::Up), 0);
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::Right), 1);
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::Down), 2);
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::Left), 3);

    // Verify WASD Keys
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::W), 0);
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::D), 1);
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::S), 2);
    EXPECT_EQ(hitSystem.getDirectionForKey(sf::Keyboard::Key::A), 3);
}
