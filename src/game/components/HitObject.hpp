#pragma once

#include "../beatmap/Beatmap.hpp"

namespace game::components {

struct HitObject {
    float hitTimeSeconds;
    float spawnTimeSeconds;
    beatmap::NodeType type;
    int direction{0};
};

} // namespace game::components
