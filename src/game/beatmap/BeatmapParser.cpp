#include "BeatmapParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <fmt/core.h>

namespace game::beatmap {

namespace {
    std::string trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }
}

std::optional<Beatmap> BeatmapParser::parse(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        fmt::print(stderr, "Failed to open beatmap file: {}\\n", filepath);
        return std::nullopt;
    }

    Beatmap beatmap;
    std::string line;
    std::string currentSection = "";

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line.starts_with("//")) {
            continue; // Skip empty lines and comments
        }

        if (line.starts_with("[") && line.ends_with("]")) {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        std::istringstream iss(line);

        if (currentSection == "Version") {
            try {
                beatmap.version = std::stoi(line);
            } catch (...) {
                fmt::print(stderr, "Failed to parse Version: {}\\n", line);
            }
        } else if (currentSection == "Metadata") {
            auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = trim(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));

                if (key == "Title") beatmap.title = value;
                else if (key == "Artist") beatmap.artist = value;
                else if (key == "AudioPath") beatmap.audioPath = value;
                else if (key == "BaseBPM") {
                    try { beatmap.baseBpm = std::stof(value); } catch (...) {}
                } else if (key == "Offset") {
                    try { beatmap.offsetSeconds = std::stof(value); } catch (...) {}
                }
            }
        } else if (currentSection == "Timing") {
            // Format: timeSeconds, bpm
            std::string timeStr, bpmStr;
            if (std::getline(iss, timeStr, ',') && std::getline(iss, bpmStr)) {
                try {
                    beatmap.timingPoints.push_back({std::stof(timeStr), std::stof(bpmStr)});
                } catch (...) {
                    fmt::print(stderr, "Failed to parse Timing Point: {}\\n", line);
                }
            }
        } else if (currentSection == "Events") {
             // Format: timeSeconds, type, parameters...
             std::string timeStr, typeStr, paramStr;
             if (std::getline(iss, timeStr, ',') && std::getline(iss, typeStr, ',')) {
                 std::getline(iss, paramStr); // Read the rest
                 try {
                     beatmap.events.push_back({std::stof(timeStr), trim(typeStr), trim(paramStr)});
                 } catch (...) {
                     fmt::print(stderr, "Failed to parse Event: {}\\n", line);
                 }
             }
        } else if (currentSection == "Nodes") {
            // Format: timeSeconds, type(0=HitCircle, 1=Slider), x, y, direction, [duration, curveX|curveY...]
            std::string timeStr, typeStr, xStr, yStr, dirStr;
            if (std::getline(iss, timeStr, ',') && std::getline(iss, typeStr, ',') && 
                std::getline(iss, xStr, ',') && std::getline(iss, yStr, ',') &&
                std::getline(iss, dirStr, ',')) {
                try {
                    Node node;
                    node.timeSeconds = std::stof(timeStr);
                    int typeInt = std::stoi(typeStr);
                    node.type = (typeInt == 1) ? NodeType::Slider : NodeType::HitCircle;
                    node.x = std::stof(xStr);
                    node.y = std::stof(yStr);
                    node.direction = std::stoi(dirStr);

                    if (node.type == NodeType::Slider) {
                        std::string durationStr;
                        if (std::getline(iss, durationStr, ',')) {
                            node.durationSeconds = std::stof(durationStr);
                        }
                        std::string pointStr;
                        while (std::getline(iss, pointStr, ',')) {
                            auto barPos = pointStr.find('|');
                            if (barPos != std::string::npos) {
                                float px = std::stof(pointStr.substr(0, barPos));
                                float py = std::stof(pointStr.substr(barPos + 1));
                                node.curvePoints.push_back({px, py});
                            }
                        }
                    }
                    beatmap.nodes.push_back(node);
                } catch (...) {
                    fmt::print(stderr, "Failed to parse Node: {}\\n", line);
                }
            }
        }
    }

    return beatmap;
}

} // namespace game::beatmap
