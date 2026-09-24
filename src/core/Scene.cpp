/**
 * @file Scene model and entity storage implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Scene.h
 */
#include "core/Scene.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace lrender
{

const Material& Entity::EntityMaterialData() const noexcept
{
    return m_entityMaterial;
}

Material& Entity::EntityMaterialData() noexcept
{
    return m_entityMaterial;
}

const Material& Entity::EffectiveMaterial() const noexcept
{
    return m_hasOverrideEntityMaterial ? m_overrideEntityMaterial : m_entityMaterial;
}

Material& Entity::EditableMaterial() noexcept
{
    if (!m_hasOverrideEntityMaterial)
    {
        m_overrideEntityMaterial = m_entityMaterial;
    }
    return m_overrideEntityMaterial;
}

bool Entity::HasMaterialOverride() const noexcept
{
    return m_hasOverrideEntityMaterial;
}

void Entity::SetOverrideMaterial(Material material)
{
    if (material.NearlyEquals(m_entityMaterial))
    {
        m_hasOverrideEntityMaterial = false;
        return;
    }
    m_overrideEntityMaterial = std::move(material);
    m_hasOverrideEntityMaterial = true;
}

void Entity::ClearMaterialOverride() noexcept
{
    m_hasOverrideEntityMaterial = false;
}

bool Entity::IsMesh() const noexcept
{
    return std::holds_alternative<MeshGeometry>(geometry);
}

bool Entity::IsSolid() const noexcept
{
    return !IsMesh();
}

PrimitiveType Entity::GetPrimitiveType() const noexcept
{
    return IsMesh() ? PrimitiveType::Mesh : std::get<SolidGeometry>(geometry).Type();
}

SolidGeometry* Entity::Solid() noexcept
{
    return std::get_if<SolidGeometry>(&geometry);
}

const SolidGeometry* Entity::Solid() const noexcept
{
    return std::get_if<SolidGeometry>(&geometry);
}

const MeshGeometry* Entity::Mesh() const noexcept
{
    return std::get_if<MeshGeometry>(&geometry);
}

const BoundingBox& Entity::LocalBoundingBoxData() const noexcept
{
    return m_localBoundingBox;
}

const BoundingBox& Entity::BoundingBoxData() const noexcept
{
    return m_boundingBox;
}

void Entity::SetLocalBoundingBox(BoundingBox boundingBox)
{
    m_localBoundingBox = std::move(boundingBox);
    CalculateBoundingBox();
}

void Entity::CalculateBoundingBox() noexcept
{
    m_boundingBox.Reset();
    m_boundingBox.ExtendBox(m_localBoundingBox, transform.ToMatrix());
}

const BoundingBox& Model::BoundingBoxData() const noexcept
{
    return m_boundingBox;
}

void Model::CalculateBoundingBoxes() noexcept
{
    m_boundingBox.Reset();
    for (Entity& entity : entities)
    {
        entity.CalculateBoundingBox();
        m_boundingBox.ExtendBox(entity.BoundingBoxData());
    }
}

Model& Scene::CreateModel(std::string name)
{
    if (name.empty())
    {
        throw std::invalid_argument("Model name must not be empty");
    }
    Model model;
    model.id = m_nextModelId++;
    model.name = std::move(name);
    m_models.push_back(std::move(model));
    return m_models.back();
}

Model& Scene::CreateMeshModel(std::filesystem::path assetPath, std::string name,
                              std::span<const std::string> assetEntityNames,
                              std::span<const BoundingBox> assetEntityBounds)
{
    if (assetPath.empty() || assetEntityNames.empty())
    {
        throw std::invalid_argument("Mesh model requires an asset path and at least one entity");
    }
    if (!assetEntityBounds.empty() && assetEntityBounds.size() != assetEntityNames.size())
    {
        throw std::invalid_argument("Mesh model bounds do not match entity names");
    }
    if (assetEntityNames.size() > std::numeric_limits<std::uint32_t>::max() ||
        std::ranges::any_of(assetEntityNames, [](const std::string& value)
        {
            return value.empty();
        }))
    {
        throw std::invalid_argument("Mesh model entity names are invalid");
    }
    const ModelId modelId = CreateModel(std::move(name)).id;
    for (std::size_t index = 0; index < assetEntityNames.size(); ++index)
    {
        const BoundingBox localBoundingBox = assetEntityBounds.empty() ? BoundingBox{} : assetEntityBounds[index];
        CreateMeshEntity(modelId, assetPath, static_cast<std::uint32_t>(index), assetEntityNames[index],
                         localBoundingBox);
    }
    return *FindModel(modelId);
}

Entity& Scene::CreateEntity(ModelId modelId, PrimitiveType primitive, std::string name)
{
    switch (primitive)
    {
    case PrimitiveType::Cube:
        return CreateSolidEntity(modelId, SolidGeometry::Cube(), std::move(name));
    case PrimitiveType::Sphere:
        return CreateSolidEntity(modelId, SolidGeometry::Sphere(), std::move(name));
    case PrimitiveType::Plane:
        return CreateSolidEntity(modelId, SolidGeometry::Plane(), std::move(name));
    case PrimitiveType::Mesh:
        throw std::invalid_argument("Use CreateMeshEntity for mesh entities");
    }
    throw std::invalid_argument("Unknown primitive type");
}

Entity& Scene::CreateSolidEntity(ModelId modelId, SolidGeometry geometry, std::string name)
{
    Entity entity;
    entity.id = m_nextEntityId++;
    entity.name = std::move(name);
    entity.SetLocalBoundingBox(geometry.LocalBoundingBox());
    entity.geometry = std::move(geometry);
    return AddEntity(modelId, std::move(entity));
}

Entity& Scene::CreateMeshEntity(ModelId modelId, std::filesystem::path assetPath, std::uint32_t assetEntityIndex,
                                std::string name, BoundingBox localBoundingBox)
{
    Entity entity;
    entity.id = m_nextEntityId++;
    entity.name = std::move(name);
    entity.EntityMaterialData().SetTextureSource(MaterialTextureSource::Source);
    entity.SetLocalBoundingBox(std::move(localBoundingBox));
    entity.geometry = MeshGeometry{std::move(assetPath), assetEntityIndex};
    return AddEntity(modelId, std::move(entity));
}

Model& Scene::AddModel(Model model)
{
    if (model.id == 0 || model.name.empty() || FindModel(model.id) != nullptr)
    {
        throw std::invalid_argument("Model snapshot is invalid or already exists");
    }
    std::unordered_set<EntityId> entityIds;
    for (Entity& entity : model.entities)
    {
        if (entity.IsSolid())
        {
            entity.SetLocalBoundingBox(entity.Solid()->LocalBoundingBox());
        }
        ValidateEntity(entity);
        if (!entityIds.insert(entity.id).second || FindEntity(entity.id) != nullptr)
        {
            throw std::invalid_argument("Model snapshot contains an existing entity id");
        }
    }
    m_nextModelId = std::max(m_nextModelId, model.id + 1);
    for (const Entity& entity : model.entities)
    {
        m_nextEntityId = std::max(m_nextEntityId, entity.id + 1);
    }
    m_models.push_back(std::move(model));
    CalculateBoundingBoxes();
    return m_models.back();
}

Entity& Scene::AddEntity(ModelId modelId, Entity entity)
{
    Model* model = FindModel(modelId);
    if (model == nullptr)
    {
        throw std::invalid_argument("Target model does not exist");
    }
    ValidateEntity(entity);
    if (FindEntity(entity.id) != nullptr)
    {
        throw std::invalid_argument("Entity id already exists");
    }
    m_nextEntityId = std::max(m_nextEntityId, entity.id + 1);
    model->entities.push_back(std::move(entity));
    CalculateBoundingBoxes();
    return model->entities.back();
}

std::optional<Model> Scene::RemoveModel(ModelId id)
{
    const auto iterator = std::find_if(m_models.begin(), m_models.end(), [id](const Model& model) { return model.id == id; });
    if (iterator == m_models.end())
    {
        return std::nullopt;
    }
    Model removed = std::move(*iterator);
    m_models.erase(iterator);
    CalculateBoundingBoxes();
    return removed;
}

std::optional<Entity> Scene::RemoveEntity(EntityId id)
{
    Model* model = FindEntityModel(id);
    if (model == nullptr)
    {
        return std::nullopt;
    }
    const auto iterator = std::find_if(
        model->entities.begin(), model->entities.end(), [id](const Entity& entity)
        {
            return entity.id == id;
        });
    Entity removed = std::move(*iterator);
    model->entities.erase(iterator);
    CalculateBoundingBoxes();
    return removed;
}

Model* Scene::FindModel(ModelId id)
{
    const auto iterator = std::find_if(m_models.begin(), m_models.end(), [id](const Model& model) { return model.id == id; });
    return iterator == m_models.end() ? nullptr : &*iterator;
}

const Model* Scene::FindModel(ModelId id) const
{
    const auto iterator = std::find_if(m_models.begin(), m_models.end(), [id](const Model& model) { return model.id == id; });
    return iterator == m_models.end() ? nullptr : &*iterator;
}

Entity* Scene::FindEntity(EntityId id)
{
    for (Model& model : m_models)
    {
        const auto iterator = std::find_if(
            model.entities.begin(), model.entities.end(), [id](const Entity& entity)
            {
                return entity.id == id;
            });
        if (iterator != model.entities.end())
        {
            return &*iterator;
        }
    }
    return nullptr;
}

const Entity* Scene::FindEntity(EntityId id) const
{
    for (const Model& model : m_models)
    {
        const auto iterator = std::find_if(
            model.entities.begin(), model.entities.end(), [id](const Entity& entity)
            {
                return entity.id == id;
            });
        if (iterator != model.entities.end())
        {
            return &*iterator;
        }
    }
    return nullptr;
}

Model* Scene::FindEntityModel(EntityId id)
{
    const auto iterator = std::find_if(
        m_models.begin(), m_models.end(), [id](const Model& model)
        {
            return std::ranges::any_of(
                model.entities, [id](const Entity& entity)
                {
                    return entity.id == id;
                });
        });
    return iterator == m_models.end() ? nullptr : &*iterator;
}

const Model* Scene::FindEntityModel(EntityId id) const
{
    const auto iterator = std::find_if(
        m_models.begin(), m_models.end(), [id](const Model& model)
        {
            return std::ranges::any_of(
                model.entities, [id](const Entity& entity)
                {
                    return entity.id == id;
                });
        });
    return iterator == m_models.end() ? nullptr : &*iterator;
}

std::size_t Scene::EntityCount() const noexcept
{
    std::size_t count = 0;
    for (const Model& model : m_models)
    {
        count += model.entities.size();
    }
    return count;
}

const std::vector<Model>& Scene::Models() const noexcept
{
    return m_models;
}

std::vector<Model>& Scene::Models() noexcept
{
    return m_models;
}

const BoundingBox& Scene::BoundingBoxData() const noexcept
{
    return m_boundingBox;
}

void Scene::CalculateBoundingBoxes() noexcept
{
    m_boundingBox.Reset();
    for (Model& model : m_models)
    {
        model.CalculateBoundingBoxes();
        m_boundingBox.ExtendBox(model.BoundingBoxData());
    }
}

void Scene::ValidateEntity(const Entity& entity)
{
    if (entity.id == 0 || entity.name.empty())
    {
        throw std::invalid_argument("Entity snapshot is invalid");
    }
    if (entity.IsMesh())
    {
        if (entity.Mesh()->assetPath.empty())
        {
            throw std::invalid_argument("Mesh entity requires an asset path");
        }
    }
    else if (entity.GetPrimitiveType() == PrimitiveType::Mesh)
    {
        throw std::invalid_argument("Solid entity cannot use the Mesh primitive type");
    }
}

} // namespace lrender
