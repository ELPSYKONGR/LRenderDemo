/**
 * @file Minimal scene model for render-lab entities.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Transform.h
 */
#pragma once

#include "core/Transform.h"

#include <SimpleMath.h>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace lrender {

enum class PrimitiveType { Cube, Sphere, Plane };

struct Entity {
    std::uint32_t id{};
    std::string name;
    PrimitiveType primitive{PrimitiveType::Cube};
    Transform transform;
    DirectX::SimpleMath::Color color{0.72F, 0.78F, 0.86F, 1.0F};
    std::filesystem::path modelPath;

    [[nodiscard]] bool IsModel() const noexcept { return !modelPath.empty(); }
};

class Scene final {
public:
    /** Creates and inserts an entity with a stable identifier. */
    Entity& CreateEntity(PrimitiveType primitive, std::string name);

    /** Creates an entity that references a cached glTF/GLB model. */
    Entity& CreateModelEntity(std::filesystem::path modelPath, std::string name);

    /** Re-inserts a prior snapshot, primarily for redo. */
    Entity& AddEntity(Entity entity);

    /** Removes an entity and returns its complete snapshot. */
    std::optional<Entity> RemoveEntity(std::uint32_t id);

    [[nodiscard]] Entity* FindEntity(std::uint32_t id);
    [[nodiscard]] const Entity* FindEntity(std::uint32_t id) const;
    [[nodiscard]] const std::vector<Entity>& Entities() const noexcept { return entities_; }

private:
    std::vector<Entity> entities_;
    std::uint32_t nextId_{1};
};

} // namespace lrender
