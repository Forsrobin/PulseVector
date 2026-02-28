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
        auto first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        auto last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
}

std::optional<Beatmap> BeatmapParser::parse(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        fmt::print(stderr, "Failed to open beatmap: {}\\n", filepath);
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return parseFromContent(buffer.str());
}

std::optional<Beatmap> BeatmapParser::parseFromContent(const std::string& content) {
    Beatmap beatmap;
    std::istringstream stream(content);
    std::string line;
    std::string currentSection = "";

    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        if (currentSection == "Version") {
            try { beatmap.version = std::stoi(line); } catch (...) {}
        } else if (currentSection == "Metadata") {
            auto parts = split(line, ':');
            if (parts.size() >= 2) {
                std::string key = trim(parts[0]);
                std::string value = trim(line.substr(line.find(':') + 1));
                if (key == "Title") beatmap.title = value;
                else if (key == "Artist") beatmap.artist = value;
                else if (key == "AudioPath") beatmap.audioPath = value;
                else if (key == "BackgroundPath") beatmap.backgroundPath = value;
                else if (key == "BaseBPM") beatmap.baseBpm = std::stof(value);
                else if (key == "Offset") beatmap.offsetSeconds = std::stof(value);
            }
        } else if (currentSection == "Timing") {
            auto parts = split(line, ',');
            if (parts.size() >= 2) {
                try {
                    beatmap.timingPoints.push_back({std::stof(parts[0]), std::stof(parts[1])});
                } catch (...) {}
            }
        } else if (currentSection == "Events") {
            auto parts = split(line, ',');
            if (parts.size() >= 3) {
                try {
                    beatmap.events.push_back({std::stof(parts[0]), parts[1], parts[2]});
                } catch (...) {}
            }
        } else if (currentSection == "Nodes") {
            auto parts = split(line, ',');
            if (parts.size() >= 5) {
                try {
                    Node node;
                    node.timeSeconds = std::stof(parts[0]);
                    int typeInt = std::stoi(parts[1]);
                    if (typeInt == 1) node.type = NodeType::Slider;
                    else if (typeInt == 2) node.type = NodeType::Wall;
                    else node.type = NodeType::HitCircle;
                    
                    node.x = std::stof(parts[2]);
                    node.y = std::stof(parts[3]);
                    node.direction = std::stoi(parts[4]);

                    if (node.type == NodeType::Slider && parts.size() >= 6) {
                        node.durationSeconds = std::stof(parts[5]);
                        for (size_t i = 6; i < parts.size(); ++i) {
                            auto p = split(parts[i], '|');
                            if (p.size() == 2) {
                                node.curvePoints.push_back({std::stof(p[0]), std::stof(p[1])});
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

std::vector<char> BeatmapParser::readFileFromZip(const std::string& archivePath, const std::string& filename) {
    int err = 0;
    zip* z = zip_open(archivePath.c_str(), 0, &err);
    if (!z) return {};

    zip_file* f = zip_fopen(z, filename.c_str(), 0);
    if (!f) {
        zip_close(z);
        return {};
    }

    zip_stat_t st;
    zip_stat_init(&st);
    zip_stat(z, filename.c_str(), 0, &st);

    std::vector<char> buffer(st.size);
    zip_fread(f, buffer.data(), st.size);
    zip_fclose(f);
    zip_close(z);

    return buffer;
}

} // namespace game::beatmap
