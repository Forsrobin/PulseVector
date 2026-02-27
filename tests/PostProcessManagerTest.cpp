#include <gtest/gtest.h>
#include "engine/graphics/PostProcessManager.hpp"
#include <vector>

TEST(PostProcessManagerTest, UpdateAudioData) {
    engine::graphics::PostProcessManager ppm(1280, 720);
    
    std::vector<float> fft(64, 0.5f);
    ppm.updateAudioData(0.8f, 0.9f, fft);
    
    // We can't easily check private members without making them public or adding accessors
    // But we can at least verify it doesn't crash
}

TEST(PostProcessManagerTest, SetBlurStrength) {
    engine::graphics::PostProcessManager ppm(1280, 720);
    ppm.setBlurStrength(5.0f);
    // Again, just verifying no crashes for now as it's a visual component
}
