#pragma once

#include "../beatmap/Beatmap.hpp"

namespace game::components {

struct HitObject {
    float hitTimeSeconds;
    float spawnTimeSeconds;
    beatmap::NodeType type;
    int direction{0};

    // Slider specific
    float durationSeconds{0.f};
    bool isSliding{false};
    float currentProgress{0.f};
    std::vector<sf::Vector2f> sliderPoints;
};

} // namespace game::components
