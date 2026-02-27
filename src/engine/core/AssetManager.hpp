#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <fmt/core.h>
#include <SFML/Graphics/Font.hpp>

namespace engine::core {

template <typename T>
class AssetManager {
public:
    std::shared_ptr<T> load(const std::string& name, const std::string& path) {
        auto it = m_assets.find(name);
        if (it != m_assets.end()) {
            return it->second;
        }

        auto asset = std::make_shared<T>();
        if constexpr (std::is_same_v<T, sf::Font>) {
            if (!asset->openFromFile(path)) {
                fmt::print(stderr, "Failed to open font: {} from {}\\n", name, path);
                return nullptr;
            }
        } else {
            if (!asset->loadFromFile(path)) {
                fmt::print(stderr, "Failed to load asset: {} from {}\\n", name, path);
                return nullptr;
            }
        }

        m_assets[name] = asset;
        return asset;
    }

    std::shared_ptr<T> get(const std::string& name) const {
        auto it = m_assets.find(name);
        if (it != m_assets.end()) {
            return it->second;
        }
        return nullptr;
    }

    void remove(const std::string& name) {
        m_assets.erase(name);
    }

    void clear() {
        m_assets.clear();
    }

private:
    std::unordered_map<std::string, std::shared_ptr<T>> m_assets;
};

} // namespace engine::core
