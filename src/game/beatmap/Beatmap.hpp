#pragma once

#include <string>
#include <vector>

namespace game::beatmap {

struct TimingPoint {
    float timeSeconds; // Time in seconds
    float bpm;
};

struct Event {
    float timeSeconds;
    std::string type;
    std::string parameters;
};

enum class NodeType {
    HitCircle,
    Slider
};

struct Node {
    float timeSeconds;
    NodeType type;
    float x;
    float y;
    int direction{0}; // 0: Up, 1: Right, 2: Down, 3: Left
    
    // Slider specific
    std::vector<std::pair<float, float>> curvePoints;
    float durationSeconds;
};

struct Beatmap {
    // Version
    int version{1};

    // Metadata
    std::string title;
    std::string artist;
    std::string audioPath;
    std::string backgroundPath;
    std::string packagePath; // Path to the .pvmap zip archive
    float baseBpm{120.0f};
    float offsetSeconds{0.0f};

    // Timing
    std::vector<TimingPoint> timingPoints;

    // Events
    std::vector<Event> events;

    // Nodes
    std::vector<Node> nodes;
};

} // namespace game::beatmap
