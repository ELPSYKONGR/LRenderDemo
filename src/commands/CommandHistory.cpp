/**
 * @file Undo and redo history implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends commands/CommandHistory.h
 */
#include "commands/CommandHistory.h"

#include <stdexcept>

namespace lrender {

CommandHistory::CommandHistory(std::size_t capacity) : capacity_(capacity) {
    if (capacity_ == 0) {
        throw std::invalid_argument("Command history capacity must be positive");
    }
}

void CommandHistory::Execute(std::unique_ptr<ICommand> command) {
    if (!command) {
        throw std::invalid_argument("Command must not be null");
    }
    command->Execute();
    Store(std::move(command));
}

void CommandHistory::PushApplied(std::unique_ptr<ICommand> command) {
    if (!command) {
        throw std::invalid_argument("Command must not be null");
    }
    Store(std::move(command));
}

void CommandHistory::Store(std::unique_ptr<ICommand> command) {
    redoStack_.clear();
    if (undoStack_.size() == capacity_) {
        undoStack_.erase(undoStack_.begin());
    }
    Entry entry{std::move(command), currentRevision_, nextRevision_++};
    currentRevision_ = entry.afterRevision;
    undoStack_.push_back(std::move(entry));
}

bool CommandHistory::Undo() {
    if (undoStack_.empty()) {
        return false;
    }
    Entry entry = std::move(undoStack_.back());
    undoStack_.pop_back();
    entry.command->Undo();
    currentRevision_ = entry.beforeRevision;
    redoStack_.push_back(std::move(entry));
    return true;
}

bool CommandHistory::Redo() {
    if (redoStack_.empty()) {
        return false;
    }
    Entry entry = std::move(redoStack_.back());
    redoStack_.pop_back();
    entry.command->Execute();
    currentRevision_ = entry.afterRevision;
    undoStack_.push_back(std::move(entry));
    return true;
}

void CommandHistory::Clear() noexcept {
    undoStack_.clear();
    redoStack_.clear();
    currentRevision_ = 0;
    savedRevision_ = 0;
    nextRevision_ = 1;
}

} // namespace lrender
