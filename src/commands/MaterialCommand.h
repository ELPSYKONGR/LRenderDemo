/**
 * @file Reversible entity material edit.
 * @author Codex
 * @created 2026-08-25
 * @depends commands/ICommand.h, core/Scene.h
 */
#pragma once

#include "commands/ICommand.h"
#include "core/Scene.h"

namespace lrender
{

class MaterialCommand final : public ICommand
{
  public:
    MaterialCommand(Scene& scene, std::uint32_t entityId, Material before, Material after);
    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string_view Name() const noexcept override;

  private:
    void Apply(const Material& value);

    Scene& m_scene;
    std::uint32_t m_entityId;
    Material m_before;
    Material m_after;
};

} // namespace lrender
