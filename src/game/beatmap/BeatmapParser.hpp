#pragma once

#include "Beatmap.hpp"
#include <string>
#include <optional>

namespace game::beatmap {

class BeatmapParser {
public:
    static std::optional<Beatmap> parse(const std::string& filepath);
};

} // namespace game::beatmap
