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
    void MarkSaved() noexcept { m_savedRevision = m_currentRevision; }
    [[nodiscard]] bool CanUndo() const noexcept { return !m_undoStack.empty(); }
    [[nodiscard]] bool CanRedo() const noexcept { return !m_redoStack.empty(); }
    [[nodiscard]] bool IsModified() const noexcept {
        return m_currentRevision != m_savedRevision;
    }

private:
    struct Entry {
        std::unique_ptr<ICommand> command;
        std::uint64_t beforeRevision{};
        std::uint64_t afterRevision{};
    };

    void Store(std::unique_ptr<ICommand> command);

    std::size_t m_capacity;
    std::vector<Entry> m_undoStack;
    std::vector<Entry> m_redoStack;
    std::uint64_t m_currentRevision{};
    std::uint64_t m_savedRevision{};
    std::uint64_t m_nextRevision{1};
};

} // namespace lrender
