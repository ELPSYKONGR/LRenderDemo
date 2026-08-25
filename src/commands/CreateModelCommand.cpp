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
    : scene_(scene), model_(std::move(model)) {}

void CreateModelCommand::Execute() { scene_.AddModel(model_); }

void CreateModelCommand::Undo() {
    auto removed = scene_.RemoveModel(model_.id);
    if (!removed) {
        throw std::runtime_error("Created model no longer exists");
    }
    model_ = std::move(*removed);
}

} // namespace lrender
