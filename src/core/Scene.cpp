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

namespace lrender {

Model& Scene::CreateModel(std::string name) {
    if (name.empty()) {
        throw std::invalid_argument("Model name must not be empty");
    }
    Model model;
    model.id = nextModelId_++;
    model.name = std::move(name);
    models_.push_back(std::move(model));
    return models_.back();
}

Model& Scene::CreateMeshModel(
    std::filesystem::path assetPath, std::string name,
    std::span<const std::string> assetEntityNames) {
    if (assetPath.empty() || assetEntityNames.empty()) {
        throw std::invalid_argument("Mesh model requires an asset path and at least one entity");
    }
    if (assetEntityNames.size() > std::numeric_limits<std::uint32_t>::max() ||
        std::ranges::any_of(assetEntityNames, [](const std::string& value) {
            return value.empty();
        })) {
        throw std::invalid_argument("Mesh model entity names are invalid");
    }
    const ModelId modelId = CreateModel(std::move(name)).id;
    for (std::size_t index = 0; index < assetEntityNames.size(); ++index) {
        CreateMeshEntity(
            modelId, assetPath, static_cast<std::uint32_t>(index), assetEntityNames[index]);
    }
    return *FindModel(modelId);
}

Entity& Scene::CreateEntity(ModelId modelId, PrimitiveType primitive, std::string name) {
    switch (primitive) {
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

Entity& Scene::CreateSolidEntity(
    ModelId modelId, SolidGeometry geometry, std::string name) {
    Entity entity;
    entity.id = nextEntityId_++;
    entity.name = std::move(name);
    entity.geometry = std::move(geometry);
    return AddEntity(modelId, std::move(entity));
}

Entity& Scene::CreateMeshEntity(
    ModelId modelId, std::filesystem::path assetPath,
    std::uint32_t assetEntityIndex, std::string name) {
    Entity entity;
    entity.id = nextEntityId_++;
    entity.name = std::move(name);
    entity.material.baseColor = {1.0F, 1.0F, 1.0F, 1.0F};
    entity.geometry = MeshGeometry{std::move(assetPath), assetEntityIndex};
    return AddEntity(modelId, std::move(entity));
}

Model& Scene::AddModel(Model model) {
    if (model.id == 0 || model.name.empty() || FindModel(model.id) != nullptr) {
        throw std::invalid_argument("Model snapshot is invalid or already exists");
    }
    std::unordered_set<EntityId> entityIds;
    for (const Entity& entity : model.entities) {
        ValidateEntity(entity);
        if (!entityIds.insert(entity.id).second || FindEntity(entity.id) != nullptr) {
            throw std::invalid_argument("Model snapshot contains an existing entity id");
        }
    }
    nextModelId_ = std::max(nextModelId_, model.id + 1);
    for (const Entity& entity : model.entities) {
        nextEntityId_ = std::max(nextEntityId_, entity.id + 1);
    }
    models_.push_back(std::move(model));
    return models_.back();
}

Entity& Scene::AddEntity(ModelId modelId, Entity entity) {
    Model* model = FindModel(modelId);
    if (model == nullptr) {
        throw std::invalid_argument("Target model does not exist");
    }
    ValidateEntity(entity);
    if (FindEntity(entity.id) != nullptr) {
        throw std::invalid_argument("Entity id already exists");
    }
    nextEntityId_ = std::max(nextEntityId_, entity.id + 1);
    model->entities.push_back(std::move(entity));
    return model->entities.back();
}

std::optional<Model> Scene::RemoveModel(ModelId id) {
    const auto iterator = std::find_if(
        models_.begin(), models_.end(), [id](const Model& model) { return model.id == id; });
    if (iterator == models_.end()) {
        return std::nullopt;
    }
    Model removed = std::move(*iterator);
    models_.erase(iterator);
    return removed;
}

std::optional<Entity> Scene::RemoveEntity(EntityId id) {
    Model* model = FindEntityModel(id);
    if (model == nullptr) {
        return std::nullopt;
    }
    const auto iterator = std::find_if(
        model->entities.begin(), model->entities.end(),
        [id](const Entity& entity) { return entity.id == id; });
    Entity removed = std::move(*iterator);
    model->entities.erase(iterator);
    return removed;
}

Model* Scene::FindModel(ModelId id) {
    const auto iterator = std::find_if(
        models_.begin(), models_.end(), [id](const Model& model) { return model.id == id; });
    return iterator == models_.end() ? nullptr : &*iterator;
}

const Model* Scene::FindModel(ModelId id) const {
    const auto iterator = std::find_if(
        models_.begin(), models_.end(), [id](const Model& model) { return model.id == id; });
    return iterator == models_.end() ? nullptr : &*iterator;
}

Entity* Scene::FindEntity(EntityId id) {
    for (Model& model : models_) {
        const auto iterator = std::find_if(
            model.entities.begin(), model.entities.end(),
            [id](const Entity& entity) { return entity.id == id; });
        if (iterator != model.entities.end()) {
            return &*iterator;
        }
    }
    return nullptr;
}

const Entity* Scene::FindEntity(EntityId id) const {
    for (const Model& model : models_) {
        const auto iterator = std::find_if(
            model.entities.begin(), model.entities.end(),
            [id](const Entity& entity) { return entity.id == id; });
        if (iterator != model.entities.end()) {
            return &*iterator;
        }
    }
    return nullptr;
}

Model* Scene::FindEntityModel(EntityId id) {
    const auto iterator = std::find_if(models_.begin(), models_.end(), [id](const Model& model) {
        return std::ranges::any_of(
            model.entities, [id](const Entity& entity) { return entity.id == id; });
    });
    return iterator == models_.end() ? nullptr : &*iterator;
}

const Model* Scene::FindEntityModel(EntityId id) const {
    const auto iterator = std::find_if(models_.begin(), models_.end(), [id](const Model& model) {
        return std::ranges::any_of(
            model.entities, [id](const Entity& entity) { return entity.id == id; });
    });
    return iterator == models_.end() ? nullptr : &*iterator;
}

std::size_t Scene::EntityCount() const noexcept {
    std::size_t count = 0;
    for (const Model& model : models_) {
        count += model.entities.size();
    }
    return count;
}

void Scene::ValidateEntity(const Entity& entity) {
    if (entity.id == 0 || entity.name.empty()) {
        throw std::invalid_argument("Entity snapshot is invalid");
    }
    if (entity.IsMesh()) {
        if (entity.Mesh()->assetPath.empty()) {
            throw std::invalid_argument("Mesh entity requires an asset path");
        }
    } else if (entity.GetPrimitiveType() == PrimitiveType::Mesh) {
        throw std::invalid_argument("Solid entity cannot use the Mesh primitive type");
    }
}

} // namespace lrender
