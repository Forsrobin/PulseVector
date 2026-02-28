#include "BeatmapWriter.hpp"
#include <fstream>
#include <sstream>
#include <fmt/core.h>

namespace game::beatmap {

bool BeatmapWriter::save(const Beatmap& beatmap, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    file << serialize(beatmap);
    return true;
}

std::string BeatmapWriter::serialize(const Beatmap& beatmap) {
    std::stringstream ss;
    
    // Version
    ss << "[Version]\n";
    ss << beatmap.version << "\n\n";

    // Metadata
    ss << "[Metadata]\n";
    ss << "Title: " << beatmap.title << "\n";
    ss << "Artist: " << beatmap.artist << "\n";
    ss << "AudioPath: " << beatmap.audioPath << "\n";
    ss << "BackgroundPath: " << beatmap.backgroundPath << "\n";
    ss << "BaseBPM: " << beatmap.baseBpm << "\n";
    ss << "Offset: " << beatmap.offsetSeconds << "\n\n";

    // Timing
    ss << "[Timing]\n";
    for (const auto& tp : beatmap.timingPoints) {
        ss << tp.timeSeconds << "," << tp.bpm << "\n";
    }
    ss << "\n";

    // Events
    ss << "[Events]\n";
    for (const auto& ev : beatmap.events) {
        ss << ev.timeSeconds << "," << ev.type << "," << ev.parameters << "\n";
    }
    ss << "\n";

    // Nodes
    ss << "[Nodes]\n";
    for (const auto& node : beatmap.nodes) {
        int typeInt = 0;
        if (node.type == NodeType::Slider) typeInt = 1;
        else if (node.type == NodeType::Wall) typeInt = 2;

        ss << node.timeSeconds << "," 
           << typeInt << ","
           << node.x << "," << node.y << ","
           << node.direction;
        
        if (node.type == NodeType::Slider) {
            ss << "," << node.durationSeconds;
            for (const auto& p : node.curvePoints) {
                ss << "," << p.x << "|" << p.y;
            }
        }
        ss << "\n";
    }

    return ss.str();
}

} // namespace game::beatmap
