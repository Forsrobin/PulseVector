#pragma once

#include "../beatmap/Beatmap.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

namespace game::states {

class EditorCommand {
public:
    virtual ~EditorCommand() = default;
    virtual void execute(game::beatmap::Beatmap& beatmap) = 0;
    virtual void undo(game::beatmap::Beatmap& beatmap) = 0;
};

class HistoryManager {
public:
    void execute(game::beatmap::Beatmap& beatmap, std::unique_ptr<EditorCommand> command) {
        command->execute(beatmap);
        m_undoStack.push_back(std::move(command));
        m_redoStack.clear();
        if (m_undoStack.size() > 50) {
            m_undoStack.erase(m_undoStack.begin());
        }
    }

    void undo(game::beatmap::Beatmap& beatmap) {
        if (m_undoStack.empty()) return;
        auto cmd = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        cmd->undo(beatmap);
        m_redoStack.push_back(std::move(cmd));
    }

    void redo(game::beatmap::Beatmap& beatmap) {
        if (m_redoStack.empty()) return;
        auto cmd = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        cmd->execute(beatmap);
        m_undoStack.push_back(std::move(cmd));
    }

    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }

private:
    std::vector<std::unique_ptr<EditorCommand>> m_undoStack;
    std::vector<std::unique_ptr<EditorCommand>> m_redoStack;
};

// Concrete Commands
class AddNodeCommand : public EditorCommand {
public:
    AddNodeCommand(game::beatmap::Node node) : m_node(std::move(node)) {}
    void execute(game::beatmap::Beatmap& beatmap) override {
        beatmap.nodes.push_back(m_node);
        std::sort(beatmap.nodes.begin(), beatmap.nodes.end(), [](const auto& a, const auto& b) {
            return a.timeSeconds < b.timeSeconds;
        });
    }
    void undo(game::beatmap::Beatmap& beatmap) override {
        auto it = std::find_if(beatmap.nodes.begin(), beatmap.nodes.end(), [this](const auto& n) {
            return std::abs(n.timeSeconds - m_node.timeSeconds) < 0.001f && std::abs(n.x - m_node.x) < 0.001f;
        });
        if (it != beatmap.nodes.end()) beatmap.nodes.erase(it);
    }
private:
    game::beatmap::Node m_node;
};

class DeleteNodeCommand : public EditorCommand {
public:
    DeleteNodeCommand(int index, game::beatmap::Node node) : m_index(index), m_node(std::move(node)) {}
    void execute(game::beatmap::Beatmap& beatmap) override {
        auto it = std::find_if(beatmap.nodes.begin(), beatmap.nodes.end(), [this](const auto& n) {
            return std::abs(n.timeSeconds - m_node.timeSeconds) < 0.001f && std::abs(n.x - m_node.x) < 0.001f;
        });
        if (it != beatmap.nodes.end()) beatmap.nodes.erase(it);
    }
    void undo(game::beatmap::Beatmap& beatmap) override {
        beatmap.nodes.push_back(m_node);
        std::sort(beatmap.nodes.begin(), beatmap.nodes.end(), [](const auto& a, const auto& b) {
            return a.timeSeconds < b.timeSeconds;
        });
    }
private:
    int m_index;
    game::beatmap::Node m_node;
};

class MoveNodeCommand : public EditorCommand {
public:
    MoveNodeCommand(game::beatmap::Node oldNode, game::beatmap::Node newNode) 
        : m_oldNode(std::move(oldNode)), m_newNode(std::move(newNode)) {}
    
    void execute(game::beatmap::Beatmap& beatmap) override {
        apply(beatmap, m_oldNode, m_newNode);
    }
    void undo(game::beatmap::Beatmap& beatmap) override {
        apply(beatmap, m_newNode, m_oldNode);
    }
private:
    void apply(game::beatmap::Beatmap& beatmap, const game::beatmap::Node& from, const game::beatmap::Node& to) {
        auto it = std::find_if(beatmap.nodes.begin(), beatmap.nodes.end(), [&from](const auto& n) {
            return std::abs(n.timeSeconds - from.timeSeconds) < 0.001f && std::abs(n.x - from.x) < 0.001f;
        });
        if (it != beatmap.nodes.end()) {
            it->x = to.x;
            it->y = to.y;
            it->timeSeconds = to.timeSeconds;
            it->curvePoints = to.curvePoints;
        }
    }
    game::beatmap::Node m_oldNode;
    game::beatmap::Node m_newNode;
};

} // namespace game::states
