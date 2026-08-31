/**
 * @file Undo support for a scene model and all of its entities.
 * @author Codex
 * @created 2026-08-25
 * @depends commands/ICommand.h, core/Scene.h
 */
#pragma once

#include "commands/ICommand.h"
#include "core/Scene.h"

namespace lrender {

class CreateModelCommand final : public ICommand {
public:
    CreateModelCommand(Scene& scene, Model model);
    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string_view Name() const noexcept override { return "Create model"; }

private:
    Scene& m_scene;
    Model m_model;
};

} // namespace lrender
