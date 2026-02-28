#pragma once

#include "engine/core/Scene.hpp"
#include "engine/core/Application.hpp"
#include "../beatmap/Beatmap.hpp"
#include "EditorHistory.hpp"
#include <SFML/Graphics.hpp>
#include <optional>

namespace game::states {

class LevelEditorScene : public engine::core::Scene {
public:
    explicit LevelEditorScene(engine::core::Application& app, game::beatmap::Beatmap map, std::string mapPath = "");
    ~LevelEditorScene() override = default;

    void onInitialize(entt::registry& registry) override;
    void handleEvent(const sf::Event& event) override;
    void update(entt::registry& registry, sf::Time dt) override;
    void render(entt::registry& registry, float interpolation) override;

private:
    void updateTimeline(sf::Time dt);
    void renderToolbar(sf::RenderTarget& target);
    void renderTimeline(sf::RenderTarget& target);
    void renderViewport(sf::RenderTarget& target);
    void renderInspector(sf::RenderTarget& target);
    void renderNodeTree(sf::RenderTarget& target);
    void renderTools(sf::RenderTarget& target);
    void drawGrid(sf::RenderTarget& target, float startX, float endX, float timelineY);

    void addNode(float time, float x, float y, game::beatmap::NodeType type);
    void deleteSelectedNode();
    void saveToDisk();
    void testPlay();

    engine::core::Application& m_app;
    game::beatmap::Beatmap m_beatmap;
    std::string m_mapPath;
    bool m_hasUnsavedChanges{false};

    HistoryManager m_history;
    game::beatmap::Node m_dragStartNode;
    bool m_isDragging{false};

    // Playback state
    float m_currentTime{0.0f};
    bool m_isPlaying{false};
    float m_playbackSpeed{1.0f};

    // Timeline UI state
    float m_zoom{100.0f}; // Pixels per second
    float m_scrollOffset{0.0f}; // Seconds from start

    // UI Layout
    sf::FloatRect m_timelineBounds;
    sf::FloatRect m_viewportBounds;
    sf::FloatRect m_inspectorBounds;
    sf::FloatRect m_treeBounds;
    sf::FloatRect m_toolBounds;

    struct ToolButton {
        std::string label;
        game::beatmap::NodeType type;
        sf::FloatRect bounds;
    };
    std::vector<ToolButton> m_toolButtons;

    // Editing state
    int m_selectedNodeIndex{-1};
    int m_selectedAnchorIndex{-1};
    float m_treeScroll{0.0f};
    sf::Vector2f m_cursorPos;
    
    int m_sliderStep{0}; // 0: Idle, 1: Placing End
    game::beatmap::Node m_tempSliderNode;

    game::beatmap::NodeType m_currentPlacementType{game::beatmap::NodeType::HitCircle};

    std::optional<sf::Text> m_infoText;
    std::optional<sf::RectangleShape> m_timelineBg;
    std::optional<sf::RectangleShape> m_viewportBg;
    std::optional<sf::RectangleShape> m_sidebarBg;
    std::optional<sf::RectangleShape> m_playhead;
};

} // namespace game::states
