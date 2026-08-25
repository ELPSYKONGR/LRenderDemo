/**
 * @file Entity material command implementation.
 * @author Codex
 * @created 2026-08-25
 * @depends commands/MaterialCommand.h
 */
#include "commands/MaterialCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender {

MaterialCommand::MaterialCommand(
    Scene& scene, std::uint32_t entityId, EntityMaterial before, EntityMaterial after)
    : scene_(scene), entityId_(entityId), before_(std::move(before)), after_(std::move(after)) {}

void MaterialCommand::Execute() { Apply(after_); }

void MaterialCommand::Undo() { Apply(before_); }

void MaterialCommand::Apply(const EntityMaterial& value) {
    Entity* entity = scene_.FindEntity(entityId_);
    if (entity == nullptr) {
        throw std::runtime_error("Material command target no longer exists");
    }
    entity->material = value;
}

} // namespace lrender
