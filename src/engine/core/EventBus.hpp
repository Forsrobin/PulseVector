#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace engine::core {

class EventBus {
public:
    template <typename T>
    using Callback = std::function<void(const T&)>;

    template <typename T>
    void subscribe(Callback<T> callback) {
        m_subscribers[typeid(T)].push_back([callback](const void* event) {
            callback(*static_cast<const T*>(event));
        });
    }

    template <typename T>
    void publish(const T& event) {
        auto it = m_subscribers.find(typeid(T));
        if (it != m_subscribers.end()) {
            for (auto& callback : it->second) {
                callback(&event);
            }
        }
    }

    void clear() {
        m_subscribers.clear();
    }

private:
    using InternalCallback = std::function<void(const void*)>;
    std::unordered_map<std::type_index, std::vector<InternalCallback>> m_subscribers;
};

} // namespace engine::core
