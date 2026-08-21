/**
 * @file Reversible entity transform edit.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/ICommand.h, core/Scene.h
 */
#pragma once

#include "commands/ICommand.h"
#include "core/Scene.h"

namespace lrender {

class TransformCommand final : public ICommand {
public:
    TransformCommand(Scene& scene, std::uint32_t entityId, Transform before, Transform after);
    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string_view Name() const noexcept override { return "Transform entity"; }

private:
    void Apply(const Transform& value);

    Scene& scene_;
    std::uint32_t entityId_;
    Transform before_;
    Transform after_;
};

} // namespace lrender
