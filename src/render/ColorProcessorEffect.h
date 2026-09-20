/**
 * @file Fullscreen color processor effect.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <filesystem>
#include <wrl/client.h>

namespace lrender
{

class ColorProcessorEffect final : public IRenderEffect
{
  public:
    ColorProcessorEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                         const std::filesystem::path& shaderDirectory);

    void SetSource(ID3D11ShaderResourceView* source);
    void BindPipeline(const EffectFrameContext& frame) override;
    void RenderEffect(const EffectFrameContext& frame) override;
    [[nodiscard]] std::string_view Name() const noexcept override;

  private:
    void SetPipeline(ID3D11DeviceContext* context);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_source;
};

} // namespace lrender
