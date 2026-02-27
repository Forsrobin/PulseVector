#pragma once

#include "Beatmap.hpp"
#include <string>
#include <optional>

namespace game::beatmap {

class BeatmapParser {
public:
    static std::optional<Beatmap> parse(const std::string& filepath);
    static std::optional<Beatmap> parseFromContent(const std::string& content);
    static std::vector<char> readFileFromZip(const std::string& archivePath, const std::string& filename);
};

} // namespace game::beatmap
