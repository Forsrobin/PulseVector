#include "BeatmapParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <fmt/core.h>
#include <zip.h>
#include <vector>

namespace game::beatmap {

namespace {
    std::string trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    bool isZipFile(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) return false;
        char signature[2];
        file.read(signature, 2);
        return signature[0] == 'P' && signature[1] == 'K';
    }
}

std::optional<Beatmap> BeatmapParser::parse(const std::string& filepath) {
    if (!isZipFile(filepath)) {
        // Fallback to legacy plain text parsing if it's not a zip
        std::ifstream file(filepath);
        if (!file.is_open()) {
            fmt::print(stderr, "Failed to open legacy beatmap file: {}\n", filepath);
            return std::nullopt;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto map = parseFromContent(buffer.str());
        if (map) map->packagePath = ""; // No package
        return map;
    }

    // Handle ZIP archive
    int err = 0;
    zip* z = zip_open(filepath.c_str(), 0, &err);
    if (!z) {
        fmt::print(stderr, "Failed to open zip archive: {}\n", filepath);
        return std::nullopt;
    }

    std::optional<Beatmap> result = std::nullopt;
    zip_int64_t num_entries = zip_get_num_entries(z, 0);
    
    for (zip_int64_t i = 0; i < num_entries; ++i) {
        const char* name = zip_get_name(z, i, 0);
        if (name && std::string(name).ends_with(".pvmap")) {
            zip_file* f = zip_fopen_index(z, i, 0);
            if (f) {
                zip_stat_t st;
                zip_stat_index(z, i, 0, &st);
                std::string content;
                content.resize(st.size);
                zip_fread(f, content.data(), st.size);
                zip_fclose(f);

                result = parseFromContent(content);
                if (result) {
                    result->packagePath = filepath;
                }
                break;
            }
        }
    }

    zip_close(z);
    return result;
}

std::vector<char> BeatmapParser::readFileFromZip(const std::string& archivePath, const std::string& filename) {
    int err = 0;
    zip* z = zip_open(archivePath.c_str(), 0, &err);
    if (!z) return {};

    std::vector<char> buffer;
    zip_file* f = zip_fopen(z, filename.c_str(), 0);
    if (f) {
        zip_stat_t st;
        zip_stat(z, filename.c_str(), 0, &st);
        buffer.resize(st.size);
        zip_fread(f, buffer.data(), st.size);
        zip_fclose(f);
    }

    zip_close(z);
    return buffer;
}

std::optional<Beatmap> BeatmapParser::parseFromContent(const std::string& content) {
    Beatmap beatmap;
    std::istringstream stream(content);
    std::string line;
    std::string currentSection = "";

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line.starts_with("//")) {
            continue;
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
                fmt::print(stderr, "Failed to parse Version: {}\n", line);
            }
        } else if (currentSection == "Metadata") {
            auto colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = trim(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));

                if (key == "Title") beatmap.title = value;
                else if (key == "Artist") beatmap.artist = value;
                else if (key == "AudioPath") beatmap.audioPath = value;
                else if (key == "BackgroundPath") beatmap.backgroundPath = value;
                else if (key == "BaseBPM") {
                    try { beatmap.baseBpm = std::stof(value); } catch (...) {}
                } else if (key == "Offset") {
                    try { beatmap.offsetSeconds = std::stof(value); } catch (...) {}
                }
            }
        } else if (currentSection == "Timing") {
            std::string timeStr, bpmStr;
            if (std::getline(iss, timeStr, ',') && std::getline(iss, bpmStr)) {
                try {
                    beatmap.timingPoints.push_back({std::stof(timeStr), std::stof(bpmStr)});
                } catch (...) {
                    fmt::print(stderr, "Failed to parse Timing Point: {}\n", line);
                }
            }
        } else if (currentSection == "Events") {
             std::string timeStr, typeStr, paramStr;
             if (std::getline(iss, timeStr, ',') && std::getline(iss, typeStr, ',')) {
                 std::getline(iss, paramStr);
                 try {
                     beatmap.events.push_back({std::stof(timeStr), trim(typeStr), trim(paramStr)});
                 } catch (...) {
                     fmt::print(stderr, "Failed to parse Event: {}\n", line);
                 }
             }
        } else if (currentSection == "Nodes") {
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
                    fmt::print(stderr, "Failed to parse Node: {}\n", line);
                }
            }
        }
    }

    return beatmap;
}

} // namespace game::beatmap
