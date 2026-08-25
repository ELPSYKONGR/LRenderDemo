/**
 * @file Entity creation command implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/CreateEntityCommand.h
 */
#include "commands/CreateEntityCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender {

CreateEntityCommand::CreateEntityCommand(Scene& scene, ModelId modelId, Entity entity)
    : scene_(scene), modelId_(modelId), entity_(std::move(entity)) {}

void CreateEntityCommand::Execute() { scene_.AddEntity(modelId_, entity_); }

void CreateEntityCommand::Undo() {
    auto removed = scene_.RemoveEntity(entity_.id);
    if (!removed) {
        throw std::runtime_error("Created entity no longer exists");
    }
    entity_ = std::move(*removed);
}

} // namespace lrender
