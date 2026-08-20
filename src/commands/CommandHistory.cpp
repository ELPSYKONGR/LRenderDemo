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
    undoStack_.push_back(std::move(command));
}

bool CommandHistory::Undo() {
    if (undoStack_.empty()) {
        return false;
    }
    auto command = std::move(undoStack_.back());
    undoStack_.pop_back();
    command->Undo();
    redoStack_.push_back(std::move(command));
    return true;
}

bool CommandHistory::Redo() {
    if (redoStack_.empty()) {
        return false;
    }
    auto command = std::move(redoStack_.back());
    redoStack_.pop_back();
    command->Execute();
    undoStack_.push_back(std::move(command));
    return true;
}

} // namespace lrender
