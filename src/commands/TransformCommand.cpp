/**
 * @file Entity transform command implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/TransformCommand.h
 */
#include "commands/TransformCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender {

TransformCommand::TransformCommand(
    Scene& scene, std::uint32_t entityId, Transform before, Transform after)
    : scene_(scene), entityId_(entityId), before_(std::move(before)), after_(std::move(after)) {}

void TransformCommand::Execute() { Apply(after_); }

void TransformCommand::Undo() { Apply(before_); }

void TransformCommand::Apply(const Transform& value) {
    Entity* entity = scene_.FindEntity(entityId_);
    if (entity == nullptr) {
        throw std::runtime_error("Transform command target no longer exists");
    }
    entity->transform = value;
}

} // namespace lrender
