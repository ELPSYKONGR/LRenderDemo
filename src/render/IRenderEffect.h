/**
 * @file Render-effect boundary used by scene drawing code.
 * @author Codex
 * @created 2026-08-20
 * @depends render/EffectContext.h
 */
#pragma once

#include "render/EffectContext.h"

#include <string_view>

namespace lrender {

class IRenderEffect {
public:
    virtual ~IRenderEffect() = default;

    /** Configures shaders, constants, input layout, and rasterizer state for one mesh. */
    virtual void Bind(
        const EffectFrameContext& frame, const EffectDrawContext& draw) = 0;

    [[nodiscard]] virtual std::string_view Name() const noexcept = 0;
};

} // namespace lrender
