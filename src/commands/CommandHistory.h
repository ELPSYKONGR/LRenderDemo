/**
 * @file Bounded undo and redo history.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/ICommand.h
 */
#pragma once

#include "commands/ICommand.h"

#include <cstddef>
#include <cstdint>
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
    void Clear() noexcept;
    void MarkSaved() noexcept { savedRevision_ = currentRevision_; }
    [[nodiscard]] bool CanUndo() const noexcept { return !undoStack_.empty(); }
    [[nodiscard]] bool CanRedo() const noexcept { return !redoStack_.empty(); }
    [[nodiscard]] bool IsModified() const noexcept {
        return currentRevision_ != savedRevision_;
    }

private:
    struct Entry {
        std::unique_ptr<ICommand> command;
        std::uint64_t beforeRevision{};
        std::uint64_t afterRevision{};
    };

    void Store(std::unique_ptr<ICommand> command);

    std::size_t capacity_;
    std::vector<Entry> undoStack_;
    std::vector<Entry> redoStack_;
    std::uint64_t currentRevision_{};
    std::uint64_t savedRevision_{};
    std::uint64_t nextRevision_{1};
};

} // namespace lrender
