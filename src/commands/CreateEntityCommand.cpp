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
    : m_scene(scene), m_modelId(modelId), m_entity(std::move(entity)) {}

void CreateEntityCommand::Execute() { m_scene.AddEntity(m_modelId, m_entity); }

void CreateEntityCommand::Undo() {
    auto removed = m_scene.RemoveEntity(m_entity.id);
    if (!removed) {
        throw std::runtime_error("Created entity no longer exists");
    }
    m_entity = std::move(*removed);
}

} // namespace lrender
