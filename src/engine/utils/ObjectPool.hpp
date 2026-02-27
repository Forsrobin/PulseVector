#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <functional>

namespace engine::utils {

struct Active {};

class ObjectPool {
public:
    explicit ObjectPool(entt::registry& registry) : m_registry(registry) {}

    entt::entity acquire() {
        entt::entity entity;
        if (m_pool.empty()) {
            entity = m_registry.create();
        } else {
            entity = m_pool.back();
            m_pool.pop_back();
        }
        
        m_registry.emplace<Active>(entity);
        return entity;
    }

    void release(entt::entity entity) {
        if (m_registry.valid(entity)) {
            m_registry.remove<Active>(entity);
            m_pool.push_back(entity);
        }
    }

    size_t available() const { return m_pool.size(); }

private:
    entt::registry& m_registry;
    std::vector<entt::entity> m_pool;
};

} // namespace engine::utils
