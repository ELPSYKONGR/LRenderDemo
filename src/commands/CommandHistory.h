/**
 * @file Bounded undo and redo history.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/ICommand.h
 */
#pragma once

#include "commands/ICommand.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace lrender {

class CommandHistory final {
public:
    explicit CommandHistory(std::size_t capacity = 128);

    /** Executes a new command and clears the redo branch. */
    void Execute(std::unique_ptr<ICommand> command);

    /** Records a command whose change has already been applied by an interactive UI. */
    void PushApplied(std::unique_ptr<ICommand> command);

    bool Undo();
    bool Redo();
    [[nodiscard]] bool CanUndo() const noexcept { return !undoStack_.empty(); }
    [[nodiscard]] bool CanRedo() const noexcept { return !redoStack_.empty(); }

private:
    void Store(std::unique_ptr<ICommand> command);

    std::size_t capacity_;
    std::vector<std::unique_ptr<ICommand>> undoStack_;
    std::vector<std::unique_ptr<ICommand>> redoStack_;
};

} // namespace lrender
