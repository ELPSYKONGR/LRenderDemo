/**
 * @file Reversible editor command contract.
 * @author Codex
 * @created 2026-08-20
 */
#pragma once

#include <string_view>

namespace lrender {

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    [[nodiscard]] virtual std::string_view Name() const noexcept = 0;
};

} // namespace lrender
