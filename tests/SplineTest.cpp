#include <gtest/gtest.h>
#include "engine/utils/Spline.hpp"
#include <vector>

TEST(SplineTest, InterpolationBounds) {
    std::vector<sf::Vector2f> points = {
        {0.f, 0.f},
        {100.f, 100.f},
        {200.f, 0.f}
    };

    auto pStart = engine::utils::Spline::interpolate(points, 0.f);
    EXPECT_NEAR(pStart.x, 0.f, 0.01f);
    EXPECT_NEAR(pStart.y, 0.f, 0.01f);

    auto pEnd = engine::utils::Spline::interpolate(points, 1.f);
    EXPECT_NEAR(pEnd.x, 200.f, 0.01f);
    EXPECT_NEAR(pEnd.y, 0.f, 0.01f);
}

TEST(SplineTest, MidPoint) {
    std::vector<sf::Vector2f> points = {
        {0.f, 0.f},
        {100.f, 100.f},
        {200.f, 0.f}
    };

    auto pMid = engine::utils::Spline::interpolate(points, 0.5f);
    // For a 3-point Catmull-Rom with p0=p1 and p2=p3 logic at ends:
    // It should be exactly the middle point p1 if t=0.5 maps to it.
    EXPECT_NEAR(pMid.x, 100.f, 0.01f);
    EXPECT_NEAR(pMid.y, 100.f, 0.01f);
}
