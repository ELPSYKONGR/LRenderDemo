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
    : scene_(scene), entityId_(entityId),
      before_(std::move(before)), after_(std::move(after)) {}

void SolidGeometryCommand::Execute() { Apply(after_); }

void SolidGeometryCommand::Undo() { Apply(before_); }

void SolidGeometryCommand::Apply(const SolidGeometry& geometry) {
    Entity* entity = scene_.FindEntity(entityId_);
    if (entity == nullptr || entity->Solid() == nullptr) {
        throw std::runtime_error("Solid geometry command target is unavailable");
    }
    entity->geometry = geometry;
}

} // namespace lrender
