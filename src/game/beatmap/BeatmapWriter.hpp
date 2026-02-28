#pragma once

#include "Beatmap.hpp"
#include <string>
#include <optional>

namespace game::beatmap {

class BeatmapWriter {
public:
    static bool save(const Beatmap& beatmap, const std::string& filepath);
    static std::string serialize(const Beatmap& beatmap);
};

} // namespace game::beatmap
