/**
 * @file Scene-owned models containing solid and imported-mesh entities.
 * @author Codex
 * @created 2026-08-20
 * @depends core/EntityMaterial.h, core/Transform.h
 */
#pragma once

#include "core/EntityMaterial.h"
#include "core/SolidGeometry.h"
#include "core/Transform.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace lrender {

using ModelId = std::uint32_t;
using EntityId = std::uint32_t;

struct MeshGeometry {
    std::filesystem::path assetPath;
    std::uint32_t assetEntityIndex{};
};

using EntityGeometry = std::variant<SolidGeometry, MeshGeometry>;

struct Entity {
    EntityId id{};
    std::string name;
    Transform transform;
    EntityMaterial material;
    EntityGeometry geometry;

    [[nodiscard]] bool IsMesh() const noexcept {
        return std::holds_alternative<MeshGeometry>(geometry);
    }
    [[nodiscard]] bool IsSolid() const noexcept { return !IsMesh(); }
    [[nodiscard]] PrimitiveType GetPrimitiveType() const noexcept {
        return IsMesh() ? PrimitiveType::Mesh : std::get<SolidGeometry>(geometry).Type();
    }
    [[nodiscard]] SolidGeometry* Solid() noexcept {
        return std::get_if<SolidGeometry>(&geometry);
    }
    [[nodiscard]] const SolidGeometry* Solid() const noexcept {
        return std::get_if<SolidGeometry>(&geometry);
    }
    [[nodiscard]] const MeshGeometry* Mesh() const noexcept {
        return std::get_if<MeshGeometry>(&geometry);
    }
};

struct Model {
    ModelId id{};
    std::string name;
    std::vector<Entity> entities;
};

class Scene final {
public:
    Model& CreateModel(std::string name);
    Model& CreateMeshModel(
        std::filesystem::path assetPath, std::string name,
        std::span<const std::string> assetEntityNames);
    Entity& CreateEntity(ModelId modelId, PrimitiveType primitive, std::string name);
    Entity& CreateSolidEntity(ModelId modelId, SolidGeometry geometry, std::string name);
    Entity& CreateMeshEntity(
        ModelId modelId, std::filesystem::path assetPath,
        std::uint32_t assetEntityIndex, std::string name);

    Model& AddModel(Model model);
    Entity& AddEntity(ModelId modelId, Entity entity);
    std::optional<Model> RemoveModel(ModelId id);
    std::optional<Entity> RemoveEntity(EntityId id);

    [[nodiscard]] Model* FindModel(ModelId id);
    [[nodiscard]] const Model* FindModel(ModelId id) const;
    [[nodiscard]] Entity* FindEntity(EntityId id);
    [[nodiscard]] const Entity* FindEntity(EntityId id) const;
    [[nodiscard]] Model* FindEntityModel(EntityId id);
    [[nodiscard]] const Model* FindEntityModel(EntityId id) const;
    [[nodiscard]] const std::vector<Model>& Models() const noexcept { return models_; }
    [[nodiscard]] std::size_t EntityCount() const noexcept;

private:
    static void ValidateEntity(const Entity& entity);

    std::vector<Model> models_;
    ModelId nextModelId_{1};
    EntityId nextEntityId_{1};
};

} // namespace lrender
