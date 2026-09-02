/**
 * @file Undo and redo history implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/CommandHistory.h
 */
#include "commands/CommandHistory.h"

#include <stdexcept>

namespace lrender
{

CommandHistory::CommandHistory(std::size_t capacity) : m_capacity(capacity)
{
    if (m_capacity == 0)
    {
        throw std::invalid_argument("Command history capacity must be positive");
    }
}

void CommandHistory::Execute(std::unique_ptr<ICommand> command)
{
    if (!command)
    {
        throw std::invalid_argument("Command must not be null");
    }
    command->Execute();
    Store(std::move(command));
}

void CommandHistory::PushApplied(std::unique_ptr<ICommand> command)
{
    if (!command)
    {
        throw std::invalid_argument("Command must not be null");
    }
    Store(std::move(command));
}

void CommandHistory::Store(std::unique_ptr<ICommand> command)
{
    m_redoStack.clear();
    if (m_undoStack.size() == m_capacity)
    {
        m_undoStack.erase(m_undoStack.begin());
    }
    Entry entry{std::move(command), m_currentRevision, m_nextRevision++};
    m_currentRevision = entry.afterRevision;
    m_undoStack.push_back(std::move(entry));
}

bool CommandHistory::Undo()
{
    if (m_undoStack.empty())
    {
        return false;
    }
    Entry entry = std::move(m_undoStack.back());
    m_undoStack.pop_back();
    entry.command->Undo();
    m_currentRevision = entry.beforeRevision;
    m_redoStack.push_back(std::move(entry));
    return true;
}

bool CommandHistory::Redo()
{
    if (m_redoStack.empty())
    {
        return false;
    }
    Entry entry = std::move(m_redoStack.back());
    m_redoStack.pop_back();
    entry.command->Execute();
    m_currentRevision = entry.afterRevision;
    m_undoStack.push_back(std::move(entry));
    return true;
}

void CommandHistory::Clear() noexcept
{
    m_undoStack.clear();
    m_redoStack.clear();
    m_currentRevision = 0;
    m_savedRevision = 0;
    m_nextRevision = 1;
}

void CommandHistory::MarkSaved() noexcept
{
    m_savedRevision = m_currentRevision;
}

bool CommandHistory::CanUndo() const noexcept
{
    return !m_undoStack.empty();
}

bool CommandHistory::CanRedo() const noexcept
{
    return !m_redoStack.empty();
}

bool CommandHistory::IsModified() const noexcept
{
    return m_currentRevision != m_savedRevision;
}

} // namespace lrender
