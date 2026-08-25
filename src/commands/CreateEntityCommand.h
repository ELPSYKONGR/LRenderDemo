/**
 * @file Undo support for an entity that was interactively created.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/ICommand.h, core/Scene.h
 */
#pragma once

#include "commands/ICommand.h"
#include "core/Scene.h"

namespace lrender {

class CreateEntityCommand final : public ICommand {
public:
    CreateEntityCommand(Scene& scene, ModelId modelId, Entity entity);
    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string_view Name() const noexcept override { return "Create entity"; }

private:
    Scene& scene_;
    ModelId modelId_;
    Entity entity_;
};

} // namespace lrender
