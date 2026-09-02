/**
 * @file Undo support for parameterized solid geometry edits.
 * @author Codex
 * @created 2026-08-26
 * @depends commands/ICommand.h, core/Scene.h
 */
#pragma once

#include "commands/ICommand.h"
#include "core/Scene.h"

namespace lrender
{

class SolidGeometryCommand final : public ICommand
{
  public:
    SolidGeometryCommand(Scene& scene, EntityId entityId, SolidGeometry before, SolidGeometry after);

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string_view Name() const noexcept override;

  private:
    void Apply(const SolidGeometry& geometry);

    Scene& m_scene;
    EntityId m_entityId;
    SolidGeometry m_before;
    SolidGeometry m_after;
};

} // namespace lrender
