#include <gtest/gtest.h>
#include "game/beatmap/BeatmapParser.hpp"
#include <fstream>

class BeatmapParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream file("test.pvmap");
        file << R"([Version]
1
[Metadata]
Title: Test Title
Artist: Test Artist
AudioPath: test.mp3
BaseBPM: 120
Offset: 0.5
[Timing]
0.0, 120
[Nodes]
1.0, 0, 100, 200, 1
2.0, 1, 300, 400, 2, 1.5, 300|500, 400|500
)";
        file.close();
    }

    void TearDown() override {
        std::remove("test.pvmap");
    }
};

TEST_F(BeatmapParserTest, ParseMetadata) {
    auto map = game::beatmap::BeatmapParser::parse("test.pvmap");
    ASSERT_TRUE(map.has_value());
    EXPECT_EQ(map->version, 1);
    EXPECT_EQ(map->title, "Test Title");
    EXPECT_EQ(map->artist, "Test Artist");
    EXPECT_EQ(map->audioPath, "test.mp3");
    EXPECT_FLOAT_EQ(map->baseBpm, 120.0f);
    EXPECT_FLOAT_EQ(map->offsetSeconds, 0.5f);
}

TEST_F(BeatmapParserTest, ParseNodes) {
    auto map = game::beatmap::BeatmapParser::parse("test.pvmap");
    ASSERT_TRUE(map.has_value());
    ASSERT_EQ(map->nodes.size(), 2);
    
    EXPECT_FLOAT_EQ(map->nodes[0].timeSeconds, 1.0f);
    EXPECT_EQ(map->nodes[0].type, game::beatmap::NodeType::HitCircle);
    EXPECT_FLOAT_EQ(map->nodes[0].x, 100.0f);
    EXPECT_FLOAT_EQ(map->nodes[0].y, 200.0f);
    EXPECT_EQ(map->nodes[0].direction, 1);
    
    EXPECT_FLOAT_EQ(map->nodes[1].timeSeconds, 2.0f);
    EXPECT_EQ(map->nodes[1].type, game::beatmap::NodeType::Slider);
    EXPECT_EQ(map->nodes[1].direction, 2);
    EXPECT_FLOAT_EQ(map->nodes[1].durationSeconds, 1.5f);
    ASSERT_EQ(map->nodes[1].curvePoints.size(), 2);
}
