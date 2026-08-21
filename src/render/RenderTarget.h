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

namespace lrender {

class RenderTarget final {
public:
    void Resize(ID3D11Device* device, std::uint32_t width, std::uint32_t height);
    void BindAndClear(ID3D11DeviceContext* context, const float clearColor[4]) const;

    [[nodiscard]] ID3D11ShaderResourceView* ShaderResourceView() const noexcept {
        return shaderResourceView_.Get();
    }
    [[nodiscard]] std::uint32_t Width() const noexcept { return width_; }
    [[nodiscard]] std::uint32_t Height() const noexcept { return height_; }

private:
    std::uint32_t width_{};
    std::uint32_t height_{};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> colorTexture_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;
};

} // namespace lrender
