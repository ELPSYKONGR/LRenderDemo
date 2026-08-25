/**
 * @file Reversible entity material edit.
 * @author Codex
 * @created 2026-08-25
 * @depends commands/ICommand.h, core/Scene.h
 */
#pragma once

#include "commands/ICommand.h"
#include "core/Scene.h"

namespace lrender {

class MaterialCommand final : public ICommand {
public:
    MaterialCommand(
        Scene& scene, std::uint32_t entityId, EntityMaterial before, EntityMaterial after);
    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string_view Name() const noexcept override { return "Edit material"; }

private:
    void Apply(const EntityMaterial& value);

    Scene& scene_;
    std::uint32_t entityId_;
    EntityMaterial before_;
    EntityMaterial after_;
};

} // namespace lrender
