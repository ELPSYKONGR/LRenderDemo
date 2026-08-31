/**
 * @file Parameterized solid geometry edit command implementation.
 * @author Codex
 * @created 2026-08-26
 * @depends commands/SolidGeometryCommand.h
 */
#include "commands/SolidGeometryCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender {

SolidGeometryCommand::SolidGeometryCommand(
    Scene& scene, EntityId entityId,
    SolidGeometry before, SolidGeometry after)
    : m_scene(scene), m_entityId(entityId),
      m_before(std::move(before)), m_after(std::move(after)) {}

void SolidGeometryCommand::Execute() { Apply(m_after); }

void SolidGeometryCommand::Undo() { Apply(m_before); }

void SolidGeometryCommand::Apply(const SolidGeometry& geometry) {
    Entity* entity = m_scene.FindEntity(m_entityId);
    if (entity == nullptr || entity->Solid() == nullptr) {
        throw std::runtime_error("Solid geometry command target is unavailable");
    }
    entity->geometry = geometry;
}

} // namespace lrender
