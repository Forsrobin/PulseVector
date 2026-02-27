#include <gtest/gtest.h>
#include "engine/utils/ObjectPool.hpp"
#include <entt/entt.hpp>

TEST(ObjectPoolTest, AcquireAndRelease) {
    entt::registry registry;
    engine::utils::ObjectPool pool(registry);

    EXPECT_EQ(pool.available(), 0);

    entt::entity e1 = pool.acquire();
    EXPECT_TRUE(registry.valid(e1));
    EXPECT_TRUE(registry.all_of<engine::utils::Active>(e1));
    EXPECT_EQ(pool.available(), 0);

    pool.release(e1);
    EXPECT_FALSE(registry.all_of<engine::utils::Active>(e1));
    EXPECT_EQ(pool.available(), 1);

    entt::entity e2 = pool.acquire();
    EXPECT_EQ(e1, e2); // Reused
    EXPECT_TRUE(registry.all_of<engine::utils::Active>(e2));
    EXPECT_EQ(pool.available(), 0);
}
