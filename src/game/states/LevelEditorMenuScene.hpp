#pragma once

#include "engine/core/Scene.hpp"
#include "engine/core/Application.hpp"
#include "../beatmap/Beatmap.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio/Sound.hpp>
#include <vector>
#include <string>
#include <optional>

namespace game::states {

struct EditorLevelEntry {
    std::string title;
    std::string artist;
    std::string mapPath;
    sf::FloatRect bounds;
};

class LevelEditorMenuScene : public engine::core::Scene {
public:
    explicit LevelEditorMenuScene(engine::core::Application& app);
    ~LevelEditorMenuScene() override = default;

    void onInitialize(entt::registry& registry) override;
    void handleEvent(const sf::Event& event) override;
    void update(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    void refreshLevelList();
    void createNewLevel();
    void openLevel(const std::string& path);
    void updateWizard(sf::Time dt);
    void renderWizard(sf::RenderTarget& target);

    enum class State {
        List,
        Wizard
    };
    State m_state{State::List};

    struct WizardData {
        std::string title;
        std::string artist;
        std::string audioPath;
        std::string backgroundPath;
        float bpm{120.0f};
        int selectedField{0}; // 0: Title, 1: Artist, 2: Audio, 3: Background, 4: BPM
        std::vector<sf::FloatRect> fieldBounds;
    } m_wizardData;

    float m_wizardScroll{0.0f};
    sf::FloatRect m_createButtonBounds;
    std::optional<sf::Text> m_createButtonText;

    engine::core::Application& m_app;
    std::vector<EditorLevelEntry> m_levels;
    int m_selectedIndex{0};

    std::optional<sf::Sprite> m_backgroundSprite;
    std::optional<sf::Text> m_menuText;
    std::optional<sf::Sound> m_hoverSound;

    sf::FloatRect m_backBounds;
    std::optional<sf::Text> m_backText;
    
    sf::FloatRect m_newLevelBounds;
    std::optional<sf::Text> m_newLevelText;

    sf::FloatRect m_exportBounds;
    std::optional<sf::Text> m_exportText;
    bool m_hasExported{false};

    struct VisualState {
        float scale{1.0f};
        float offset{0.0f};
    };
    std::vector<VisualState> m_visualStates;
};

} // namespace game::states
