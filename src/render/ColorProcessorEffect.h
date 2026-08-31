/**
 * @file Fullscreen color processor effect.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <CommonStates.h>
#include <filesystem>
#include <memory>
#include <wrl/client.h>

namespace lrender {

class ColorProcessorEffect final : public IRenderEffect {
public:
    ColorProcessorEffect(ID3D11Device* device, const std::filesystem::path& shaderDirectory);

    void Bind(
        const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    void Apply(ID3D11DeviceContext* context, ID3D11ShaderResourceView* source);
    [[nodiscard]] std::string_view Name() const noexcept override { return "Color Processor"; }

private:
    void SetPipeline(ID3D11DeviceContext* context);

    std::unique_ptr<DirectX::CommonStates> m_states;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
};

} // namespace lrender
