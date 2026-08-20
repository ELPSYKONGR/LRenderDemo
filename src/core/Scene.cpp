/**
 * @file Scene entity storage implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Scene.h
 */
#include "core/Scene.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace lrender {

Entity& Scene::CreateEntity(PrimitiveType primitive, std::string name) {
    if (name.empty()) {
        throw std::invalid_argument("Entity name must not be empty");
    }

    Entity entity;
    entity.id = nextId_++;
    entity.name = std::move(name);
    entity.primitive = primitive;
    entities_.push_back(std::move(entity));
    return entities_.back();
}

Entity& Scene::AddEntity(Entity entity) {
    if (entity.id == 0 || entity.name.empty()) {
        throw std::invalid_argument("Entity snapshot is invalid");
    }
    if (FindEntity(entity.id) != nullptr) {
        throw std::invalid_argument("Entity id already exists");
    }

    nextId_ = std::max(nextId_, entity.id + 1);
    entities_.push_back(std::move(entity));
    return entities_.back();
}

std::optional<Entity> Scene::RemoveEntity(std::uint32_t id) {
    const auto iterator = std::find_if(
        entities_.begin(), entities_.end(), [id](const Entity& entity) { return entity.id == id; });
    if (iterator == entities_.end()) {
        return std::nullopt;
    }

    Entity removed = std::move(*iterator);
    entities_.erase(iterator);
    return removed;
}

Entity* Scene::FindEntity(std::uint32_t id) {
    const auto iterator = std::find_if(
        entities_.begin(), entities_.end(), [id](const Entity& entity) { return entity.id == id; });
    return iterator == entities_.end() ? nullptr : &*iterator;
}

const Entity* Scene::FindEntity(std::uint32_t id) const {
    const auto iterator = std::find_if(
        entities_.begin(), entities_.end(), [id](const Entity& entity) { return entity.id == id; });
    return iterator == entities_.end() ? nullptr : &*iterator;
}

} // namespace lrender
