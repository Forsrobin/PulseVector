#include "LevelEditorMenuScene.hpp"
#include "MenuScene.hpp"
#include "LevelEditorScene.hpp"
#include "../beatmap/BeatmapWriter.hpp"
#include "../beatmap/BeatmapParser.hpp"
#include <fmt/core.h>
#include <filesystem>
#include <cstdio>
#include <array>
#include <algorithm>

namespace fs = std::filesystem;

namespace game::states {

namespace {
    std::string pickFile(const std::string& title, const std::string& filter) {
        std::string command = fmt::format("zenity --file-selection --title=\"{}\" --file-filter=\"{}\"", title, filter);
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        if (!result.empty() && result.back() == '\n') result.pop_back();
        return result;
    }
}

LevelEditorMenuScene::LevelEditorMenuScene(engine::core::Application& app)
    : m_app(app) {
}

void LevelEditorMenuScene::onInitialize(entt::registry& registry) {
    auto bgTex = m_app.getTextureManager().get("background_1");
    if (bgTex) {
        m_backgroundSprite.emplace(*bgTex);
        sf::Vector2u texSize = bgTex->getSize();
        m_backgroundSprite->setScale({1280.f / texSize.x, 720.f / texSize.y});
        m_backgroundSprite->setColor(sf::Color(255, 255, 255, 120));
    }

    auto font = m_app.getFontManager().get("default");
    if (font) {
        m_menuText.emplace(*font, "");
        m_menuText->setCharacterSize(36);
        
        m_backText.emplace(*font, "Back");
        m_backText->setCharacterSize(40);
        auto backBounds = m_backText->getLocalBounds();
        m_backText->setOrigin({0.f, backBounds.size.y / 2.f});
        m_backText->setPosition({50.f, 670.f});
        m_backBounds = sf::FloatRect({50.f, 670.f - backBounds.size.y/2.f}, backBounds.size);

        m_newLevelText.emplace(*font, "NEW LEVEL");
        m_newLevelText->setCharacterSize(40);
        auto newBounds = m_newLevelText->getLocalBounds();
        m_newLevelText->setOrigin({newBounds.size.x, newBounds.size.y / 2.f});
        m_newLevelText->setPosition({1230.f, 670.f});
        m_newLevelBounds = sf::FloatRect({1230.f - newBounds.size.x, 670.f - newBounds.size.y/2.f}, newBounds.size);

        m_exportText.emplace(*font, "EXPORT");
        m_exportText->setCharacterSize(30);
        auto expBounds = m_exportText->getLocalBounds();
        m_exportText->setOrigin({expBounds.size.x / 2.f, expBounds.size.y / 2.f});
        m_exportText->setPosition({640.f, 670.f});
        m_exportBounds = sf::FloatRect({640.f - expBounds.size.x/2.f, 670.f - expBounds.size.y/2.f}, expBounds.size);

        m_createButtonText.emplace(*font, "CREATE LEVEL");
        m_createButtonText->setCharacterSize(40);
    }

    refreshLevelList();
}

void LevelEditorMenuScene::refreshLevelList() {
    m_levels.clear();
    if (fs::exists("edits") && fs::is_directory("edits")) {
        for (auto const& entry : fs::directory_iterator("edits")) {
            if (entry.path().extension() == ".pvmap") {
                m_levels.push_back({entry.path().stem().string(), "Custom Artist", entry.path().string(), {}});
            }
        }
    }
    m_visualStates.resize(m_levels.size(), {1.0f, 0.0f});
    m_selectedIndex = std::min(m_selectedIndex, static_cast<int>(m_levels.size()) - 1);
    if (m_selectedIndex < 0 && !m_levels.empty()) m_selectedIndex = 0;
}

void LevelEditorMenuScene::handleEvent(const sf::Event& event) {
    if (m_state == State::Wizard) {
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            auto unicode = textEntered->unicode;
            if (unicode < 128 && unicode >= 32) {
                char c = static_cast<char>(unicode);
                if (m_wizardData.selectedField < 4) {
                    std::string* targetField = nullptr;
                    switch (m_wizardData.selectedField) {
                        case 0: targetField = &m_wizardData.title; break;
                        case 1: targetField = &m_wizardData.artist; break;
                        case 2: targetField = &m_wizardData.audioPath; break;
                        case 3: targetField = &m_wizardData.backgroundPath; break;
                        default: break;
                    }
                    if (targetField) (*targetField) += c;
                } else if (m_wizardData.selectedField == 4) {
                    if ((c >= '0' && c <= '9') || c == '.') {
                        static std::string tempBpm = "";
                        tempBpm += c;
                        try { m_wizardData.bpm = std::stof(tempBpm); } catch(...) {}
                    }
                }
            }
        } else if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
            auto key = keyPress->code;
            if (key == sf::Keyboard::Key::Backspace) {
                if (m_wizardData.selectedField < 4) {
                    std::string* targetField = nullptr;
                    switch (m_wizardData.selectedField) {
                        case 0: targetField = &m_wizardData.title; break;
                        case 1: targetField = &m_wizardData.artist; break;
                        case 2: targetField = &m_wizardData.audioPath; break;
                        case 3: targetField = &m_wizardData.backgroundPath; break;
                        default: break;
                    }
                    if (targetField && !targetField->empty()) targetField->pop_back();
                }
            } else if (key == sf::Keyboard::Key::Tab) {
                m_wizardData.selectedField = (m_wizardData.selectedField + 1) % 5;
            } else if (key == sf::Keyboard::Key::Up) {
                m_wizardData.selectedField = (m_wizardData.selectedField - 1 + 5) % 5;
            } else if (key == sf::Keyboard::Key::Down) {
                m_wizardData.selectedField = (m_wizardData.selectedField + 1) % 5;
            } else if (key == sf::Keyboard::Key::Enter) {
                if (!m_wizardData.title.empty()) createNewLevel();
            } else if (key == sf::Keyboard::Key::Escape) {
                m_state = State::List;
            }
        } else if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
            m_wizardScroll -= scroll->delta * 30.f;
            m_wizardScroll = std::clamp(m_wizardScroll, 0.f, 400.f); 
        } else if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePress->button == sf::Mouse::Button::Left) {
                auto mousePos = m_app.getWindow().mapPixelToCoords(mousePress->position);
                bool clickedField = false;
                for (int i = 0; i < (int)m_wizardData.fieldBounds.size(); ++i) {
                    if (m_wizardData.fieldBounds[i].contains(mousePos)) {
                        m_wizardData.selectedField = i;
                        clickedField = true;
                        if (i == 2) {
                            std::string picked = pickFile("Select Audio File", "*.mp3 *.ogg *.wav");
                            if (!picked.empty()) m_wizardData.audioPath = picked;
                        } else if (i == 3) {
                            std::string picked = pickFile("Select Background Image", "*.png *.jpg *.jpeg");
                            if (!picked.empty()) m_wizardData.backgroundPath = picked;
                        }
                        break;
                    }
                }
                if (!clickedField && m_createButtonBounds.contains(mousePos)) {
                    if (!m_wizardData.title.empty()) {
                        createNewLevel();
                    }
                }
            }
        }
    } else {
        if (const auto* keyPress = event.getIf<sf::Event::KeyPressed>()) {
            auto key = keyPress->code;
            if (key == sf::Keyboard::Key::Escape) {
                m_app.transitionToScene(std::make_unique<MenuScene>(m_app));
            } else if (key == sf::Keyboard::Key::Up) {
                if (!m_levels.empty()) m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_levels.size())) % m_levels.size();
            } else if (key == sf::Keyboard::Key::Down) {
                if (!m_levels.empty()) m_selectedIndex = (m_selectedIndex + 1) % m_levels.size();
            } else if (key == sf::Keyboard::Key::Enter && !m_levels.empty()) {
                openLevel(m_levels[m_selectedIndex].mapPath);
            }
        }
    }
}

void LevelEditorMenuScene::update(entt::registry& registry, sf::Time dt) {
    float elapsed = dt.asSeconds();
    auto mousePos = m_app.getInputSystem().getMousePosition();

    if (m_state == State::List) {
        if (m_backBounds.contains(mousePos)) {
            if (m_backText) m_backText->setFillColor(sf::Color::Cyan);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                m_app.transitionToScene(std::make_unique<MenuScene>(m_app));
            }
        } else {
            if (m_backText) m_backText->setFillColor(sf::Color::White);
        }

        if (m_newLevelBounds.contains(mousePos)) {
            if (m_newLevelText) m_newLevelText->setFillColor(sf::Color::Cyan);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                m_state = State::Wizard;
                m_wizardScroll = 0.f;
            }
        } else {
            if (m_newLevelText) m_newLevelText->setFillColor(sf::Color::White);
        }

        if (!m_levels.empty() && m_exportBounds.contains(mousePos)) {
            if (m_exportText) m_exportText->setFillColor(sf::Color::Yellow);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !m_hasExported) {
                fs::path src = m_levels[m_selectedIndex].mapPath;
                fs::path dest = fs::path("levels") / src.filename();
                try {
                    fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
                    m_hasExported = true;
                    if (m_exportText) m_exportText->setString("EXPORTED!");
                } catch (...) {
                     if (m_exportText) m_exportText->setString("FAILED!");
                }
            }
        } else {
            if (m_exportText) m_exportText->setFillColor(sf::Color::White);
        }

        for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
            if (m_levels[i].bounds.contains(mousePos)) {
                if (m_selectedIndex != i) {
                    m_selectedIndex = i;
                    m_hasExported = false;
                    if (m_exportText) m_exportText->setString("EXPORT");
                }
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    openLevel(m_levels[i].mapPath);
                }
            }
        }

        for (int i = 0; i < static_cast<int>(m_visualStates.size()); ++i) {
            float targetScale = (i == m_selectedIndex) ? 1.15f : 1.0f;
            float targetOffset = (i == m_selectedIndex) ? 40.f : 0.f;
            m_visualStates[i].scale += (targetScale - m_visualStates[i].scale) * 15.f * elapsed;
            m_visualStates[i].offset += (targetOffset - m_visualStates[i].offset) * 15.f * elapsed;
        }
    }
}

void LevelEditorMenuScene::createNewLevel() {
    game::beatmap::Beatmap map;
    map.title = m_wizardData.title;
    map.artist = m_wizardData.artist;
    map.audioPath = m_wizardData.audioPath;
    map.backgroundPath = m_wizardData.backgroundPath;
    map.baseBpm = m_wizardData.bpm;
    
    std::string safeName = m_wizardData.title;
    std::replace(safeName.begin(), safeName.end(), ' ', '_');
    
    // Ensure edits folder exists
    if (!fs::exists("edits")) {
        fs::create_directories("edits");
    }
    
    std::string path = "edits/" + safeName + ".pvmap";
    
    fmt::print("Saving new level to: {}\n", path);
    if (game::beatmap::BeatmapWriter::save(map, path)) {
        fmt::print("Transitioning to LevelEditorScene...\n");
        m_app.transitionToScene(std::make_unique<LevelEditorScene>(m_app, std::move(map), path));
    } else {
        fmt::print(stderr, "Failed to save level file at: {}!\n", path);
    }
}

void LevelEditorMenuScene::openLevel(const std::string& path) {
    auto map = game::beatmap::BeatmapParser::parse(path);
    if (map) {
        m_app.transitionToScene(std::make_unique<LevelEditorScene>(m_app, std::move(*map), path));
    }
}

void LevelEditorMenuScene::render(entt::registry& registry, float interpolation) {
    auto& target = m_app.getRenderTarget();
    if (m_backgroundSprite) target.draw(*m_backgroundSprite);

    if (m_state == State::List) {
        auto font = m_app.getFontManager().get("default");
        if (font) {
            sf::Text title(*font, "LEVEL EDITOR", 70);
            title.setFillColor(sf::Color(255, 100, 255));
            auto titleBounds = title.getLocalBounds();
            title.setOrigin({titleBounds.size.x / 2.f, titleBounds.size.y / 2.f});
            title.setPosition({640.f, 80.f});
            target.draw(title);
        }

        if (m_backText) target.draw(*m_backText);
        if (m_newLevelText) target.draw(*m_newLevelText);
        if (!m_levels.empty() && m_exportText) target.draw(*m_exportText);

        if (!m_menuText) return;

        float startX = 200.f;
        float startY = 220.f;
        float spacingY = 85.f;
        float diagonalStepX = 30.f;

        for (int i = 0; i < static_cast<int>(m_levels.size()); ++i) {
            auto& level = m_levels[i];
            const auto& visual = m_visualStates[i];
            float posX = startX + (i * diagonalStepX) + visual.offset;
            float posY = startY + (i * spacingY);
            float width = 600.f;
            float height = 70.f;
            level.bounds = sf::FloatRect({posX, posY}, {width * visual.scale, height * visual.scale});
            sf::RectangleShape rect({width, height});
            rect.setOrigin({0.f, height / 2.f});
            rect.setPosition({posX, posY});
            rect.setScale({visual.scale, visual.scale});
            bool isSelected = (i == m_selectedIndex);
            rect.setFillColor(isSelected ? sf::Color(255, 0, 255, 100) : sf::Color(60, 40, 60, 180));
            rect.setOutlineThickness(isSelected ? 3.f : 1.f);
            rect.setOutlineColor(isSelected ? sf::Color::White : sf::Color(100, 100, 100, 150));
            target.draw(rect);
            m_menuText->setString(fmt::format("{} - {}", level.artist, level.title));
            m_menuText->setCharacterSize(static_cast<unsigned int>(32 * visual.scale));
            auto textBounds = m_menuText->getLocalBounds();
            m_menuText->setOrigin({0.f, textBounds.size.y / 2.f});
            m_menuText->setPosition({posX + 30.f * visual.scale, posY});
            m_menuText->setFillColor(isSelected ? sf::Color::White : sf::Color(180, 180, 180));
            target.draw(*m_menuText);
        }
    } else {
        renderWizard(target);
    }
}

void LevelEditorMenuScene::renderWizard(sf::RenderTarget& target) {
    auto font = m_app.getFontManager().get("default");
    if (!font) return;

    sf::Text title(*font, "NEW LEVEL WIZARD", 60);
    title.setFillColor(sf::Color::Cyan);
    title.setPosition({100.f, 50.f - m_wizardScroll});
    target.draw(title);

    std::vector<std::pair<std::string, std::string>> fields = {
        {"Song Title", m_wizardData.title},
        {"Artist", m_wizardData.artist},
        {"Audio File", m_wizardData.audioPath},
        {"Background Image", m_wizardData.backgroundPath},
        {"Base BPM", fmt::format("{:.2f}", m_wizardData.bpm)}
    };

    m_wizardData.fieldBounds.resize(fields.size());

    for (int i = 0; i < static_cast<int>(fields.size()); ++i) {
        float y = 200.f + i * 100.f - m_wizardScroll;
        
        sf::Text label(*font, fields[i].first, 24);
        label.setFillColor(sf::Color(200, 200, 200));
        label.setPosition({150.f, y});
        target.draw(label);

        sf::RectangleShape rect({800.f, 50.f});
        rect.setPosition({150.f, y + 30.f});
        rect.setFillColor(sf::Color(30, 30, 30, 200));
        rect.setOutlineThickness(m_wizardData.selectedField == i ? 3.f : 1.f);
        rect.setOutlineColor(m_wizardData.selectedField == i ? sf::Color::Cyan : sf::Color(100, 100, 100));
        target.draw(rect);

        m_wizardData.fieldBounds[i] = sf::FloatRect(rect.getPosition(), rect.getSize());

        sf::View oldView = target.getView();
        sf::FloatRect clipRect({160.f, y + 35.f}, {780.f, 40.f});
        sf::Vector2u windowSize = target.getSize();
        sf::FloatRect viewport({clipRect.position.x / windowSize.x, clipRect.position.y / windowSize.y}, 
                               {clipRect.size.x / windowSize.x, clipRect.size.y / windowSize.y});

        sf::View clipView(clipRect);
        clipView.setViewport(viewport);
        target.setView(clipView);

        sf::Text value(*font, fields[i].second, 30);
        value.setFillColor(sf::Color::White);
        value.setPosition(clipRect.position);
        auto valBounds = value.getLocalBounds();
        if (valBounds.size.x > 780.f) {
            value.setPosition({clipRect.position.x + 780.f - valBounds.size.x, clipRect.position.y});
        }
        target.draw(value);
        target.setView(oldView);
    }

    float btnY = 200.f + fields.size() * 100.f + 50.f - m_wizardScroll;
    sf::RectangleShape createBtn({400.f, 80.f});
    createBtn.setOrigin({200.f, 40.f});
    createBtn.setPosition({640.f, btnY});
    
    auto mousePos = m_app.getInputSystem().getMousePosition();
    m_createButtonBounds = sf::FloatRect({640.f - 200.f, btnY - 40.f}, {400.f, 80.f});
    bool hovered = m_createButtonBounds.contains(mousePos);
    
    createBtn.setFillColor(hovered ? sf::Color(0, 200, 0, 255) : sf::Color(0, 150, 0, 200));
    createBtn.setOutlineThickness(hovered ? 4.f : 2.f);
    createBtn.setOutlineColor(sf::Color::White);
    target.draw(createBtn);

    if (m_createButtonText) {
        m_createButtonText->setPosition({640.f, btnY});
        auto b = m_createButtonText->getLocalBounds();
        m_createButtonText->setOrigin({b.size.x/2.f, b.size.y/2.f});
        m_createButtonText->setFillColor(hovered ? sf::Color::White : sf::Color(200, 200, 200));
        target.draw(*m_createButtonText);
    }

    sf::Text hint(*font, "SCROLL to navigate | TAB to switch | CLICK to select/browse", 20);
    hint.setFillColor(sf::Color(150, 150, 150));
    hint.setPosition({150.f, 680.f});
    target.draw(hint);
}

} // namespace game::states
