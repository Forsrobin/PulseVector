#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cmath>

namespace engine::utils {

class Spline {
public:
    static sf::Vector2f interpolate(const std::vector<sf::Vector2f>& points, float t) {
        if (points.empty()) return {0.f, 0.f};
        if (points.size() == 1) return points[0];
        
        if (t <= 0.f) return points.front();
        if (t >= 1.f) return points.back();

        // Map t [0, 1] to the number of segments
        float segmentT = t * static_cast<float>(points.size() - 1);
        size_t index = static_cast<size_t>(segmentT);
        float localT = segmentT - static_cast<float>(index);

        // Indices for Catmull-Rom (p0, p1, p2, p3)
        size_t p0 = (index == 0) ? 0 : index - 1;
        size_t p1 = index;
        size_t p2 = (index + 1 >= points.size()) ? points.size() - 1 : index + 1;
        size_t p3 = (index + 2 >= points.size()) ? points.size() - 1 : index + 2;

        return catmullRom(points[p0], points[p1], points[p2], points[p3], localT);
    }

private:
    static sf::Vector2f catmullRom(sf::Vector2f p0, sf::Vector2f p1, sf::Vector2f p2, sf::Vector2f p3, float t) {
        float t2 = t * t;
        float t3 = t2 * t;

        return 0.5f * (
            (2.f * p1) +
            (-p0 + p2) * t +
            (2.f * p0 - 5.f * p1 + 4.f * p2 - p3) * t2 +
            (-p0 + 3.f * p1 - 3.f * p2 + p3) * t3
        );
    }
};

} // namespace engine::utils
