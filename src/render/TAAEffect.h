/**
 * @file Temporal anti-aliasing accumulation effect.
 */
#pragma once

#include "render/Dx11ConstantBuffer.h"
#include "render/EffectResource.h"
#include "render/IRenderEffect.h"

#include <SimpleMath.h>
#include <cstdint>
#include <filesystem>

namespace lrender
{

struct alignas(16) TAAConstants
{
    DirectX::SimpleMath::Vector4 parameters;
};

class TAAEffect final : public IRenderEffect
{
  public:
    TAAEffect(ID3D11Device* device, ID3D11DeviceContext* context, const std::filesystem::path& shaderDirectory);

    void SetSource(ID3D11ShaderResourceView* source);
    void ResetHistory() noexcept;
    void BindPipeline(const EffectFrameContext& frame) override;
    void RenderEffect(const EffectFrameContext& frame) override;
    void ResizeResources(std::uint32_t width, std::uint32_t height) override;
    [[nodiscard]] std::string_view Name() const noexcept override;
    [[nodiscard]] const EffectResource& OutputResource() const noexcept;

  private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Dx11ConstantBuffer<TAAConstants> m_constantBuffer;
    EffectResource m_historyResources[2];
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_source;
    std::uint32_t m_historyIndex = 0;
    bool m_historyValid = false;
};

} // namespace lrender
