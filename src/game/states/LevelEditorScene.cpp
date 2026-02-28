#include "LevelEditorScene.hpp"
#include "LevelEditorMenuScene.hpp"
#include "MainScene.hpp"
#include "../beatmap/BeatmapWriter.hpp"
#include "engine/utils/Spline.hpp"
#include <fmt/core.h>
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

namespace game::states {

LevelEditorScene::LevelEditorScene(engine::core::Application& app, game::beatmap::Beatmap map, std::string mapPath)
    : m_app(app), m_beatmap(std::move(map)), m_mapPath(std::move(mapPath)) {
}

void LevelEditorScene::onInitialize(entt::registry& registry) {
    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_infoText.emplace(*font, "");
        m_infoText->setCharacterSize(20);
        m_infoText->setPosition({20.f, 15.f});
    }

    m_timelineBounds = sf::FloatRect({0.f, 600.f}, {1280.f, 120.f});
    m_timelineBg.emplace(sf::Vector2f(m_timelineBounds.size));
    m_timelineBg->setPosition(m_timelineBounds.position);
    m_timelineBg->setFillColor(sf::Color(25, 25, 35, 230));

    m_viewportBounds = sf::FloatRect({220.f, 60.f}, {800.f, 520.f});
    m_viewportBg.emplace(sf::Vector2f(m_viewportBounds.size));
    m_viewportBg->setPosition(m_viewportBounds.position);
    m_viewportBg->setFillColor(sf::Color(10, 10, 15, 255));
    m_viewportBg->setOutlineThickness(2.f);
    m_viewportBg->setOutlineColor(sf::Color(60, 60, 80));

    m_treeBounds = sf::FloatRect({1030.f, 60.f}, {240.f, 520.f});
    m_sidebarBg.emplace(sf::Vector2f(m_treeBounds.size));
    m_sidebarBg->setPosition(m_treeBounds.position);
    m_sidebarBg->setFillColor(sf::Color(20, 20, 30, 200));

    m_toolBounds = sf::FloatRect({10.f, 60.f}, {200.f, 520.f});

    m_playhead.emplace(sf::Vector2f(2.f, m_timelineBounds.size.y));
    m_playhead->setFillColor(sf::Color::Cyan);

    m_toolButtons = {
        {"CIRCLE", game::beatmap::NodeType::HitCircle, {}},
        {"SLIDER", game::beatmap::NodeType::Slider, {}},
        {"WALL", game::beatmap::NodeType::Wall, {}}
    };

    if (!m_beatmap.audioPath.empty()) {
        std::string fullPath = m_beatmap.audioPath;
        if (!m_app.getAudioCore().loadMusic(fullPath)) {
            m_app.getAudioCore().loadMusic("assets/audio/" + m_beatmap.audioPath);
        }
    }
}

void LevelEditorScene::handleEvent(const sf::Event& event) {
    bool ctrl = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);

    if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
        auto key = keyPress->code;
        if (key == sf::Keyboard::Key::Escape) {
            m_app.getAudioCore().stop();
            m_app.transitionToScene(std::make_unique<LevelEditorMenuScene>(m_app));
        } else if (ctrl && key == sf::Keyboard::Key::S) {
            saveToDisk();
        } else if (ctrl && key == sf::Keyboard::Key::P) {
            testPlay();
        } else if (ctrl && key == sf::Keyboard::Key::Z) {
            m_history.undo(m_beatmap);
            m_hasUnsavedChanges = true;
        } else if (ctrl && key == sf::Keyboard::Key::Y) {
            m_history.redo(m_beatmap);
            m_hasUnsavedChanges = true;
        } else if (key == sf::Keyboard::Key::Space) {
            m_isPlaying = !m_isPlaying;
            if (m_isPlaying) {
                m_app.getAudioCore().setPlaybackPosition(sf::seconds(m_currentTime));
                m_app.getAudioCore().play();
            } else {
                m_app.getAudioCore().pause();
                m_currentTime = m_app.getAudioCore().getSmoothedPosition().asSeconds();
            }
        } else if (key == sf::Keyboard::Key::Delete || key == sf::Keyboard::Key::Backspace) {
            deleteSelectedNode();
        } else if (m_selectedNodeIndex != -1) {
            auto& node = m_beatmap.nodes[m_selectedNodeIndex];
            if (key == sf::Keyboard::Key::W) node.direction = 0;
            else if (key == sf::Keyboard::Key::D) node.direction = 1;
            else if (key == sf::Keyboard::Key::S) node.direction = 2;
            else if (key == sf::Keyboard::Key::A) node.direction = 3;
            m_hasUnsavedChanges = true;
        }
    } else if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
        auto mousePos = m_app.getInputSystem().getMousePosition();
        if (m_treeBounds.contains(mousePos)) {
            m_treeScroll -= scroll->delta * 20.f;
            m_treeScroll = std::max(0.f, m_treeScroll);
        } else if (ctrl) {
            float factor = (scroll->delta > 0) ? 1.2f : 0.8f;
            m_zoom = std::clamp(m_zoom * factor, 10.f, 2000.f);
        } else {
            m_scrollOffset -= scroll->delta * (50.f / m_zoom);
            m_scrollOffset = std::max(0.f, m_scrollOffset);
        }
    } else if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePress->button == sf::Mouse::Button::Left) {
            auto mousePos = m_app.getWindow().mapPixelToCoords(mousePress->position);
            
            for (auto& btn : m_toolButtons) {
                if (btn.bounds.contains(mousePos)) {
                    m_currentPlacementType = btn.type;
                    m_sliderStep = 0;
                    return;
                }
            }

            if (m_treeBounds.contains(mousePos)) {
                float relativeY = mousePos.y - m_treeBounds.position.y + m_treeScroll;
                int clickedIndex = static_cast<int>(relativeY / 30.f);
                if (clickedIndex >= 0 && clickedIndex < (int)m_beatmap.nodes.size()) {
                    m_selectedNodeIndex = clickedIndex;
                    m_selectedAnchorIndex = -1;
                    m_currentTime = m_beatmap.nodes[clickedIndex].timeSeconds;
                    if (m_isPlaying) m_app.getAudioCore().setPlaybackPosition(sf::seconds(m_currentTime));
                }
                return;
            }

            if (m_viewportBounds.contains(mousePos)) {
                float vx = mousePos.x - m_viewportBounds.position.x;
                float vy = mousePos.y - m_viewportBounds.position.y;

                if (m_selectedNodeIndex != -1 && m_beatmap.nodes[m_selectedNodeIndex].type == game::beatmap::NodeType::Slider) {
                    auto& node = m_beatmap.nodes[m_selectedNodeIndex];
                    for (int i = 0; i < (int)node.curvePoints.size(); ++i) {
                        float dx = vx - node.curvePoints[i].x;
                        float dy = vy - node.curvePoints[i].y;
                        if (std::sqrt(dx*dx + dy*dy) < 15.f) {
                            m_selectedAnchorIndex = i;
                            m_isDragging = true;
                            return;
                        }
                    }
                    if (ctrl) {
                        node.curvePoints.push_back({vx, vy});
                        m_hasUnsavedChanges = true;
                        return;
                    }
                }

                int foundIdx = -1;
                for (int i = 0; i < (int)m_beatmap.nodes.size(); ++i) {
                    const auto& node = m_beatmap.nodes[i];
                    if (std::abs(node.timeSeconds - m_currentTime) < 0.5f) {
                        float dx = vx - node.x;
                        float dy = vy - node.y;
                        if (std::sqrt(dx*dx + dy*dy) < 30.f) {
                            foundIdx = i;
                            break;
                        }
                    }
                }

                if (foundIdx != -1) {
                    m_selectedNodeIndex = foundIdx;
                    m_selectedAnchorIndex = -1;
                    m_dragStartNode = m_beatmap.nodes[foundIdx];
                    m_isDragging = true;
                } else {
                    if (m_currentPlacementType == game::beatmap::NodeType::Slider) {
                        if (m_sliderStep == 0) {
                            m_tempSliderNode.x = vx;
                            m_tempSliderNode.y = vy;
                            m_tempSliderNode.timeSeconds = m_currentTime;
                            m_sliderStep = 1;
                        } else {
                            if (m_currentTime > m_tempSliderNode.timeSeconds) {
                                m_tempSliderNode.type = game::beatmap::NodeType::Slider;
                                m_tempSliderNode.durationSeconds = m_currentTime - m_tempSliderNode.timeSeconds;
                                m_tempSliderNode.curvePoints = {{m_tempSliderNode.x, m_tempSliderNode.y}, {vx, vy}};
                                addNode(m_tempSliderNode.timeSeconds, m_tempSliderNode.x, m_tempSliderNode.y, game::beatmap::NodeType::Slider);
                                m_beatmap.nodes[m_selectedNodeIndex].durationSeconds = m_tempSliderNode.durationSeconds;
                                m_beatmap.nodes[m_selectedNodeIndex].curvePoints = m_tempSliderNode.curvePoints;
                                m_sliderStep = 0;
                            }
                        }
                    } else {
                        addNode(m_currentTime, vx, vy, m_currentPlacementType);
                        m_sliderStep = 0;
                    }
                }
            }
        }
    } else if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseRelease->button == sf::Mouse::Button::Left && m_isDragging) {
            m_isDragging = false;
            if (m_selectedAnchorIndex == -1 && m_selectedNodeIndex != -1) {
                auto& node = m_beatmap.nodes[m_selectedNodeIndex];
                if (node.x != m_dragStartNode.x || node.y != m_dragStartNode.y) {
                    auto cmd = std::make_unique<MoveNodeCommand>(m_dragStartNode, node);
                    m_history.execute(m_beatmap, std::move(cmd));
                    m_hasUnsavedChanges = true;
                }
            }
        }
    }
}

void LevelEditorScene::update(entt::registry& registry, sf::Time dt) {
    if (m_isPlaying) {
        m_currentTime = m_app.getAudioCore().getSmoothedPosition().asSeconds();
        float playheadX = (m_currentTime - m_scrollOffset) * m_zoom;
        if (playheadX > 900.f) m_scrollOffset = m_currentTime - (900.f / m_zoom);
        else if (playheadX < 100.f) m_scrollOffset = std::max(0.f, m_currentTime - (100.f / m_zoom));
    }

    auto mousePos = m_app.getInputSystem().getMousePosition();
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && m_timelineBounds.contains(mousePos)) {
        float timelineX = mousePos.x - m_timelineBounds.position.x;
        m_currentTime = m_scrollOffset + (timelineX / m_zoom);
        m_currentTime = std::max(0.f, std::min(m_currentTime, m_app.getAudioCore().getTotalDuration().asSeconds()));
        if (m_isPlaying) m_app.getAudioCore().setPlaybackPosition(sf::seconds(m_currentTime));
    }

    if (m_isDragging && m_selectedNodeIndex != -1) {
        float vx = std::clamp(mousePos.x - m_viewportBounds.position.x, 0.f, m_viewportBounds.size.x);
        float vy = std::clamp(mousePos.y - m_viewportBounds.position.y, 0.f, m_viewportBounds.size.y);
        
        auto& node = m_beatmap.nodes[m_selectedNodeIndex];
        if (m_selectedAnchorIndex != -1) {
            node.curvePoints[m_selectedAnchorIndex] = {vx, vy};
            if (m_selectedAnchorIndex == 0) {
                node.x = vx;
                node.y = vy;
            }
        } else {
            float dx = vx - node.x;
            float dy = vy - node.y;
            node.x = vx;
            node.y = vy;
            for (auto& p : node.curvePoints) {
                p.x += dx;
                p.y += dy;
            }
        }
        m_hasUnsavedChanges = true;
    }
}

void LevelEditorScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    target.clear(sf::Color(15, 15, 20));

    if (m_infoText) {
        m_infoText->setString(fmt::format("{}{} | {:.3f}s | {}", 
                             m_beatmap.title, m_hasUnsavedChanges ? "*" : "", m_currentTime,
                             m_sliderStep == 1 ? "SET SLIDER END" : "EDITING"));
        target.draw(*m_infoText);
    }

    renderToolbar(target);
    renderTools(target);
    renderViewport(target);
    renderNodeTree(target);
    renderTimeline(target);
}

void LevelEditorScene::renderToolbar(sf::RenderTarget& target) {
    auto font = m_app.getFontManager().get("default");
    if (!font) return;
    sf::Text help(*font, "WASD: Set Direction | Ctrl+S: Save | Ctrl+P: Test | Ctrl+Click: Add Slider Anchor", 14);
    help.setFillColor(sf::Color(120, 120, 140));
    help.setPosition({220.f, 15.f});
    target.draw(help);
}

void LevelEditorScene::renderTools(sf::RenderTarget& target) {
    auto font = m_app.getFontManager().get("default");
    if (!font) return;

    for (int i = 0; i < (int)m_toolButtons.size(); ++i) {
        auto& btn = m_toolButtons[i];
        float y = m_toolBounds.position.y + i * 60.f;
        btn.bounds = sf::FloatRect({m_toolBounds.position.x, y}, {m_toolBounds.size.x, 50.f});

        bool isActive = (m_currentPlacementType == btn.type);
        sf::RectangleShape rect(btn.bounds.size);
        rect.setPosition(btn.bounds.position);
        rect.setFillColor(isActive ? sf::Color(0, 150, 255, 100) : sf::Color(40, 40, 50, 150));
        rect.setOutlineThickness(2.f);
        rect.setOutlineColor(isActive ? sf::Color::Cyan : sf::Color(80, 80, 100));
        target.draw(rect);

        sf::Text txt(*font, btn.label, 20);
        auto b = txt.getLocalBounds();
        txt.setOrigin({b.size.x/2.f, b.size.y/2.f});
        txt.setPosition(btn.bounds.position + btn.bounds.size / 2.f);
        target.draw(txt);
    }
}

void LevelEditorScene::renderViewport(sf::RenderTarget& target) {
    if (m_viewportBg) target.draw(*m_viewportBg);

    for (int i = 0; i < (int)m_beatmap.nodes.size(); ++i) {
        const auto& node = m_beatmap.nodes[i];
        float timeDiff = node.timeSeconds - m_currentTime;
        
        if (node.type == game::beatmap::NodeType::Wall) {
            if (timeDiff > 0 && timeDiff < 2.0f) {
                float factor = timeDiff / 2.0f;
                float size = 100.f + factor * 400.f;
                sf::RectangleShape wallRect({size, size});
                wallRect.setOrigin({size/2.f, size/2.f});
                wallRect.setPosition({m_viewportBounds.position.x + node.x, m_viewportBounds.position.y + node.y});
                wallRect.setFillColor(sf::Color::Transparent);
                wallRect.setOutlineThickness(5.f);
                wallRect.setOutlineColor(sf::Color(255, 0, 0, static_cast<std::uint8_t>((1.0f-factor) * 255)));
                target.draw(wallRect);
            }
        }

        if (std::abs(timeDiff) < 1.0f) {
            float alpha = 1.0f - (std::abs(timeDiff) / 1.0f);
            
            if (node.type == game::beatmap::NodeType::Slider && !node.curvePoints.empty()) {
                sf::VertexArray splinePath(sf::PrimitiveType::LineStrip, 30);
                for (size_t p = 0; p < 30; ++p) {
                    float t = static_cast<float>(p) / 29.f;
                    sf::Vector2f pos = engine::utils::Spline::interpolate(node.curvePoints, t);
                    splinePath[p].position = m_viewportBounds.position + pos;
                    splinePath[p].color = sf::Color(255, 255, 0, static_cast<std::uint8_t>(alpha * 150));
                }
                target.draw(splinePath);

                if (i == m_selectedNodeIndex) {
                    for (auto& cp : node.curvePoints) {
                        sf::CircleShape anchor(6.f);
                        anchor.setOrigin({6.f, 6.f});
                        anchor.setPosition(m_viewportBounds.position + cp);
                        anchor.setFillColor(sf::Color::White);
                        target.draw(anchor);
                    }
                }
            }

            sf::CircleShape shape(25.f);
            shape.setOrigin({25.f, 25.f});
            shape.setPosition({m_viewportBounds.position.x + node.x, m_viewportBounds.position.y + node.y});
            
            sf::Color color;
            if (node.type == game::beatmap::NodeType::Slider) color = sf::Color::Yellow;
            else if (node.type == game::beatmap::NodeType::Wall) color = sf::Color::Red;
            else color = sf::Color::Cyan;

            color.a = static_cast<std::uint8_t>(alpha * 255.f);
            shape.setFillColor(sf::Color::Transparent);
            shape.setOutlineThickness(i == m_selectedNodeIndex ? 4.f : 2.f);
            shape.setOutlineColor(i == m_selectedNodeIndex ? sf::Color::White : color);
            target.draw(shape);

            float angle = node.direction * 90.f - 90.f;
            sf::RectangleShape dirInd({15.f, 4.f});
            dirInd.setOrigin({0.f, 2.f});
            dirInd.setPosition(shape.getPosition());
            dirInd.setRotation(sf::degrees(angle));
            dirInd.setFillColor(sf::Color(255, 255, 255, color.a));
            target.draw(dirInd);
        }
    }
}

void LevelEditorScene::renderNodeTree(sf::RenderTarget& target) {
    if (m_sidebarBg) target.draw(*m_sidebarBg);
    auto font = m_app.getFontManager().get("default");
    if (!font) return;

    sf::Text title(*font, "NODE TREE", 18);
    title.setPosition({m_treeBounds.position.x + 10.f, m_treeBounds.position.y - 30.f});
    target.draw(title);

    sf::View oldView = target.getView();
    sf::FloatRect clipRect = m_treeBounds;
    sf::Vector2u windowSize = target.getSize();
    sf::FloatRect viewport({clipRect.position.x / windowSize.x, clipRect.position.y / windowSize.y}, 
                           {clipRect.size.x / windowSize.x, clipRect.size.y / windowSize.y});
    sf::View clipView(clipRect);
    clipView.setViewport(viewport);
    target.setView(clipView);

    for (int i = 0; i < (int)m_beatmap.nodes.size(); ++i) {
        const auto& node = m_beatmap.nodes[i];
        float y = m_treeBounds.position.y + i * 30.f - m_treeScroll;
        
        sf::RectangleShape itemRect({m_treeBounds.size.x - 10.f, 25.f});
        itemRect.setPosition({m_treeBounds.position.x + 5.f, y});
        itemRect.setFillColor(i == m_selectedNodeIndex ? sf::Color(100, 100, 255, 100) : sf::Color::Transparent);
        target.draw(itemRect);

        std::string typeStr = "Circle";
        if (node.type == game::beatmap::NodeType::Slider) typeStr = "Slider";
        else if (node.type == game::beatmap::NodeType::Wall) typeStr = "Wall";

        sf::Text txt(*font, fmt::format("{:02d} | {:.2f}s | {}", i, node.timeSeconds, typeStr), 14);
        txt.setPosition({m_treeBounds.position.x + 10.f, y + 2.f});
        txt.setFillColor(std::abs(node.timeSeconds - m_currentTime) < 0.1f ? sf::Color::Yellow : sf::Color::White);
        target.draw(txt);
    }
    target.setView(oldView);
}

void LevelEditorScene::renderTimeline(sf::RenderTarget& target) {
    if (m_timelineBg) target.draw(*m_timelineBg);
    float timelineY = m_timelineBounds.position.y;
    float timelineHeight = m_timelineBounds.size.y;
    float viewWidth = m_timelineBounds.size.x;
    float startSeconds = m_scrollOffset;
    float endSeconds = startSeconds + (viewWidth / m_zoom);
    drawGrid(target, startSeconds, endSeconds, timelineY);

    for (int i = 0; i < (int)m_beatmap.nodes.size(); ++i) {
        const auto& node = m_beatmap.nodes[i];
        if (node.timeSeconds >= startSeconds && node.timeSeconds <= endSeconds) {
            float x = (node.timeSeconds - m_scrollOffset) * m_zoom;
            sf::RectangleShape marker({4.f, timelineHeight * 0.5f});
            marker.setOrigin({2.f, marker.getSize().y / 2.f});
            marker.setPosition({x, timelineY + timelineHeight / 2.f});
            
            sf::Color color;
            if (node.type == game::beatmap::NodeType::Slider) color = sf::Color::Yellow;
            else if (node.type == game::beatmap::NodeType::Wall) color = sf::Color::Red;
            else color = sf::Color::Cyan;

            if (i == m_selectedNodeIndex) color = sf::Color::White;
            marker.setFillColor(color);
            target.draw(marker);
        }
    }

    float playheadX = (m_currentTime - m_scrollOffset) * m_zoom;
    if (playheadX >= 0 && playheadX <= viewWidth) {
        m_playhead->setPosition({playheadX, timelineY});
        target.draw(*m_playhead);
    }
}

void LevelEditorScene::drawGrid(sf::RenderTarget& target, float startSeconds, float endSeconds, float timelineY) {
    auto font = m_app.getFontManager().get("default");
    float bpm = m_beatmap.baseBpm;
    float beatDuration = 60.f / bpm;
    float firstBeat = std::ceil(startSeconds / beatDuration) * beatDuration;

    float labelInterval = 1.0f;
    if (m_zoom < 50.f) labelInterval = 5.0f;
    if (m_zoom < 20.f) labelInterval = 10.0f;
    if (m_zoom > 200.f) labelInterval = 0.5f;

    for (float t = firstBeat; t <= endSeconds; t += beatDuration) {
        float x = (t - m_scrollOffset) * m_zoom;
        sf::RectangleShape line({1.f, m_timelineBounds.size.y * 0.6f});
        line.setPosition({x, timelineY + m_timelineBounds.size.y * 0.2f});
        line.setFillColor(sf::Color(100, 100, 100, 80));
        target.draw(line);

        if (std::fmod(t + 0.001f, labelInterval) < beatDuration * 0.5f && font) {
            sf::Text label(*font, fmt::format("{:.1f}s", t), 12);
            auto b = label.getLocalBounds();
            label.setOrigin({b.size.x/2.f, 0.f});
            label.setPosition({x, timelineY + 5.f});
            label.setFillColor(sf::Color(180, 180, 180));
            target.draw(label);
        }
    }
}

void LevelEditorScene::addNode(float time, float x, float y, game::beatmap::NodeType type) {
    game::beatmap::Node node;
    node.timeSeconds = time;
    node.x = x;
    node.y = y;
    node.type = type;
    node.direction = 0;
    if (type == game::beatmap::NodeType::Slider) {
        node.curvePoints = {{x, y}, {x + 100.f, y}};
        node.durationSeconds = 1.0f;
    }
    m_history.execute(m_beatmap, std::make_unique<AddNodeCommand>(node));
    m_hasUnsavedChanges = true;
    for (int i = 0; i < (int)m_beatmap.nodes.size(); ++i) {
        if (std::abs(m_beatmap.nodes[i].timeSeconds - time) < 0.001f && std::abs(m_beatmap.nodes[i].x - x) < 0.001f) {
            m_selectedNodeIndex = i;
            break;
        }
    }
}

void LevelEditorScene::deleteSelectedNode() {
    if (m_selectedNodeIndex != -1) {
        m_history.execute(m_beatmap, std::make_unique<DeleteNodeCommand>(m_selectedNodeIndex, m_beatmap.nodes[m_selectedNodeIndex]));
        m_selectedNodeIndex = -1;
        m_hasUnsavedChanges = true;
    }
}

void LevelEditorScene::saveToDisk() {
    if (m_mapPath.empty()) {
        std::string safeName = m_beatmap.title;
        std::replace(safeName.begin(), safeName.end(), ' ', '_');
        m_mapPath = "edits/" + safeName + ".pvmap";
    }
    if (game::beatmap::BeatmapWriter::save(m_beatmap, m_mapPath)) {
        m_hasUnsavedChanges = false;
        fmt::print("Saved map to {}\n", m_mapPath);
    }
}

void LevelEditorScene::testPlay() {
    m_app.getAudioCore().stop();
    m_app.transitionToScene(std::make_unique<MainScene>(m_app, m_beatmap));
}

} // namespace game::states
