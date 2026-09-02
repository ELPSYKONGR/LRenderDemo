/**
 * @file Entity transform command implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/TransformCommand.h
 */
#include "commands/TransformCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender
{

TransformCommand::TransformCommand(Scene& scene, std::uint32_t entityId, Transform before, Transform after)
    : m_scene(scene), m_entityId(entityId), m_before(std::move(before)), m_after(std::move(after))
{
}

void TransformCommand::Execute()
{
    Apply(m_after);
}

void TransformCommand::Undo()
{
    Apply(m_before);
}

void TransformCommand::Apply(const Transform& value)
{
    Entity* entity = m_scene.FindEntity(m_entityId);
    if (entity == nullptr)
    {
        throw std::runtime_error("Transform command target no longer exists");
    }
    entity->transform = value;
}

std::string_view TransformCommand::Name() const noexcept
{
    return "Transform entity";
}

} // namespace lrender
