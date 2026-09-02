/**
 * @file Resizable offscreen color/depth target displayed by ImGui.
 * @author Codex
 * @created 2026-08-20
 * @depends D3D11
 */
#pragma once

#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

namespace lrender
{

class RenderTarget final
{
  public:
    void Resize(ID3D11Device* device, std::uint32_t width, std::uint32_t height);
    void Reset() noexcept;
    void BindAndClear(ID3D11DeviceContext* context, const float clearColor[4],
                      const RenderTarget* additionalTarget = nullptr) const;

    [[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const noexcept;
    [[nodiscard]] ID3D11RenderTargetView* GetRenderTargetView() const noexcept;
    [[nodiscard]] std::uint32_t GetWidth() const noexcept;
    [[nodiscard]] std::uint32_t GetHeight() const noexcept;

  private:
    std::uint32_t m_width = 0;
    std::uint32_t m_height = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_colorTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
};

} // namespace lrender
