#include <gtest/gtest.h>
#include "engine/core/AssetManager.hpp"
#include <SFML/Graphics/Texture.hpp>

TEST(AssetManagerTest, LoadAndGet) {
    engine::core::AssetManager<sf::Texture> manager;
    
    // We don't have a real texture to load in a unit test easily without assets
    // but we can test the map logic
    auto texture = manager.get("nonexistent");
    EXPECT_EQ(texture, nullptr);
}

TEST(AssetManagerTest, Clear) {
    engine::core::AssetManager<sf::Texture> manager;
    manager.clear();
    EXPECT_EQ(manager.get("anything"), nullptr);
}
