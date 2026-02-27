#include <gtest/gtest.h>
#include "engine/core/Application.hpp"

class MockApp : public engine::core::Application {
public:
    MockApp() : engine::core::Application("Test", 800, 600) {}
protected:
    void onInitialize() override {}
    void onUpdate(sf::Time) override {}
    void onFixedUpdate(sf::Time) override {}
    void onRender(float) override {}
    void onShutdown() override {}
};

TEST(ApplicationTest, VolumeControl) {
    MockApp app;
    app.setVolume(50.f);
    EXPECT_NEAR(app.getVolume(), 50.f, 0.01f);
    
    app.setVolume(150.f); // Clamp to 100
    EXPECT_NEAR(app.getVolume(), 100.f, 0.01f);
    
    app.setVolume(-10.f); // Clamp to 0
    EXPECT_NEAR(app.getVolume(), 0.f, 0.01f);
}
