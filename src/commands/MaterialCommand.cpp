/**
 * @file Entity material command implementation.
 * @author Codex
 * @created 2026-08-25
 * @depends commands/MaterialCommand.h
 */
#include "commands/MaterialCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender
{

MaterialCommand::MaterialCommand(Scene& scene, std::uint32_t entityId, EntityMaterial before, EntityMaterial after)
    : m_scene(scene), m_entityId(entityId), m_before(std::move(before)), m_after(std::move(after))
{
}

void MaterialCommand::Execute()
{
    Apply(m_after);
}

void MaterialCommand::Undo()
{
    Apply(m_before);
}

void MaterialCommand::Apply(const EntityMaterial& value)
{
    Entity* entity = m_scene.FindEntity(m_entityId);
    if (entity == nullptr)
    {
        throw std::runtime_error("Material command target no longer exists");
    }
    entity->SetOverrideMaterial(value);
}

std::string_view MaterialCommand::Name() const noexcept
{
    return "Edit material";
}

} // namespace lrender
