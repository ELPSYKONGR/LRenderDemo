/**
 * @file Effect-owned TextureCube resource and optional render-target views.
 * @author Codex
 * @created 2026-09-07
 * @depends D3D11, DirectXTK DDSTextureLoader
 */
#pragma once

#include <array>
#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <wrl/client.h>

namespace lrender
{

class EffectCubeMapResource final
{
  public:
    EffectCubeMapResource() = default;

    EffectCubeMapResource(const EffectCubeMapResource&) = delete;
    EffectCubeMapResource& operator=(const EffectCubeMapResource&) = delete;
    EffectCubeMapResource(EffectCubeMapResource&&) noexcept = default;
    EffectCubeMapResource& operator=(EffectCubeMapResource&&) noexcept = default;

    void LoadDDS(ID3D11Device* device, const std::filesystem::path& path,
                 bool createRenderTargetView = false, bool createFaceRenderTargetViews = false,
                 bool forceSrgb = true);
    void Create(ID3D11Device* device, std::uint32_t size, DXGI_FORMAT format,
                bool createRenderTargetView = true, bool createFaceRenderTargetViews = true);
    void Reset() noexcept;

    [[nodiscard]] ID3D11Texture2D* GetTexture() const noexcept;
    [[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const noexcept;
    [[nodiscard]] ID3D11RenderTargetView* GetRenderTargetView() const noexcept;
    [[nodiscard]] ID3D11RenderTargetView* GetFaceRenderTargetView(std::uint32_t face) const noexcept;
    [[nodiscard]] std::uint32_t GetSize() const noexcept;
    [[nodiscard]] bool IsValid() const noexcept;

  private:
    void CreateRenderTargetViews(ID3D11Device* device, DXGI_FORMAT format, bool createRenderTargetView,
                                 bool createFaceRenderTargetViews);

    std::uint32_t m_size = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, 6> m_faceRenderTargetViews;
};

} // namespace lrender
