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

namespace lrender
{

class CommandHistory final
{
  public:
    explicit CommandHistory(std::size_t capacity = 128);

    /** Executes a new command and clears the redo branch. */
    void Execute(std::unique_ptr<ICommand> command);

    /** Records a command whose change has already been applied by an interactive UI. */
    void PushApplied(std::unique_ptr<ICommand> command);

    bool Undo();
    bool Redo();
    void Clear() noexcept;
    void MarkSaved() noexcept;
    [[nodiscard]] bool CanUndo() const noexcept;
    [[nodiscard]] bool CanRedo() const noexcept;
    [[nodiscard]] bool IsModified() const noexcept;

  private:
    struct Entry
    {
        std::unique_ptr<ICommand> command;
        std::uint64_t beforeRevision = 0;
        std::uint64_t afterRevision = 0;
    };

    void Store(std::unique_ptr<ICommand> command);

    std::size_t m_capacity;
    std::vector<Entry> m_undoStack;
    std::vector<Entry> m_redoStack;
    std::uint64_t m_currentRevision = 0;
    std::uint64_t m_savedRevision = 0;
    std::uint64_t m_nextRevision = 1;
};

} // namespace lrender
