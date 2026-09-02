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
#include <utility>

namespace lrender
{

using ModelId = std::uint32_t;
using EntityId = std::uint32_t;

struct MeshGeometry
{
    std::filesystem::path assetPath;
    std::uint32_t assetEntityIndex = 0;
};

using EntityGeometry = std::variant<SolidGeometry, MeshGeometry>;

struct Entity
{
    EntityId id = 0;
    std::string name;
    Transform transform;
    EntityGeometry geometry;

    [[nodiscard]] const EntityMaterial& EntityMaterialData() const noexcept;
    [[nodiscard]] EntityMaterial& EntityMaterialData() noexcept;
    [[nodiscard]] const EntityMaterial& EffectiveMaterial() const noexcept;
    [[nodiscard]] EntityMaterial& EditableMaterial() noexcept;
    [[nodiscard]] bool HasMaterialOverride() const noexcept;
    void SetOverrideMaterial(EntityMaterial material);
    void ClearMaterialOverride() noexcept;
    [[nodiscard]] bool IsMesh() const noexcept;
    [[nodiscard]] bool IsSolid() const noexcept;
    [[nodiscard]] PrimitiveType GetPrimitiveType() const noexcept;
    [[nodiscard]] SolidGeometry* Solid() noexcept;
    [[nodiscard]] const SolidGeometry* Solid() const noexcept;
    [[nodiscard]] const MeshGeometry* Mesh() const noexcept;

  private:
    EntityMaterial m_entityMaterial = EntityMaterial();
    EntityMaterial m_overrideEntityMaterial = EntityMaterial();
    bool m_hasOverrideEntityMaterial = false;
};

struct Model
{
    ModelId id = 0;
    std::string name;
    std::vector<Entity> entities;
};

class Scene final
{
  public:
    Model& CreateModel(std::string name);
    Model& CreateMeshModel(std::filesystem::path assetPath, std::string name,
                           std::span<const std::string> assetEntityNames);
    Entity& CreateEntity(ModelId modelId, PrimitiveType primitive, std::string name);
    Entity& CreateSolidEntity(ModelId modelId, SolidGeometry geometry, std::string name);
    Entity& CreateMeshEntity(ModelId modelId, std::filesystem::path assetPath, std::uint32_t assetEntityIndex,
                             std::string name);

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
    [[nodiscard]] const std::vector<Model>& Models() const noexcept;
    [[nodiscard]] std::size_t EntityCount() const noexcept;

  private:
    static void ValidateEntity(const Entity& entity);

    std::vector<Model> m_models;
    ModelId m_nextModelId = 1;
    EntityId m_nextEntityId = 1;
};

} // namespace lrender
