/**
 * @file Scene model creation command implementation.
 * @author Codex
 * @created 2026-08-25
 * @depends commands/CreateModelCommand.h
 */
#include "commands/CreateModelCommand.h"

#include <stdexcept>
#include <utility>

namespace lrender {

CreateModelCommand::CreateModelCommand(Scene& scene, Model model)
    : m_scene(scene), m_model(std::move(model)) {}

void CreateModelCommand::Execute() { m_scene.AddModel(m_model); }

void CreateModelCommand::Undo() {
    auto removed = m_scene.RemoveModel(m_model.id);
    if (!removed) {
        throw std::runtime_error("Created model no longer exists");
    }
    m_model = std::move(*removed);
}

} // namespace lrender
