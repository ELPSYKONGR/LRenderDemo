/**
 * @file Render-effect boundary used by scene drawing code.
 * @author Codex
 * @created 2026-08-20
 * @depends render/EffectContext.h
 */
#pragma once

#include "render/EffectContext.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <filesystem>
#include <string_view>
#include <wrl/client.h>

namespace lrender {

class IRenderEffect {
public:
    explicit IRenderEffect(ID3D11Device* device);
    virtual ~IRenderEffect() = default;

    IRenderEffect(const IRenderEffect&) = delete;
    IRenderEffect& operator=(const IRenderEffect&) = delete;

    virtual void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) = 0;

    [[nodiscard]] virtual std::string_view Name() const noexcept = 0;

protected:
    [[nodiscard]] ID3D11Device* Device() const noexcept { return m_device; }
    [[nodiscard]] static Microsoft::WRL::ComPtr<ID3DBlob> LoadShader(
        const std::filesystem::path& path);

private:
    ID3D11Device* m_device{};
};

} // namespace lrender
