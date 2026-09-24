/**
 * @file Effect resource factory and DX11 pipeline-state manager.
 */
#pragma once

#include "render/EffectCubeMapResource.h"
#include "render/EffectResource.h"

#include <CommonStates.h>
#include <array>
#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <wrl/client.h>

namespace lrender
{

enum class DepthMode
{
    Disabled,
    ReadOnly,
    ReadWrite
};

enum class BlendMode
{
    Opaque,
    AlphaBlend,
    Additive,
    Premultiplied
};

enum class RasterizerMode
{
    SolidCullClockwise,
    SolidCullNone,
    WireframeCullClockwise,
    WireframeCullNone,
    SolidCullClockwiseScissor
};

struct StencilDescription
{
    bool enabled = false;
    D3D11_COMPARISON_FUNC comparison = D3D11_COMPARISON_ALWAYS;
    std::uint8_t reference = 0;
    std::uint8_t readMask = 0xff;
    std::uint8_t writeMask = 0xff;
    D3D11_STENCIL_OP failOperation = D3D11_STENCIL_OP_KEEP;
    D3D11_STENCIL_OP depthFailOperation = D3D11_STENCIL_OP_KEEP;
    D3D11_STENCIL_OP passOperation = D3D11_STENCIL_OP_KEEP;
};

class EffectManager final
{
  public:
    static void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    static void Shutdown() noexcept;
    [[nodiscard]] static EffectManager& Instance();

    EffectManager(const EffectManager&) = delete;
    EffectManager& operator=(const EffectManager&) = delete;

    [[nodiscard]] ID3D11Device* Device() const noexcept;
    [[nodiscard]] ID3D11DeviceContext* Context() const noexcept;
    [[nodiscard]] std::unique_ptr<EffectResource> CreateEffectResource(
        std::uint32_t width, std::uint32_t height,
        EffectResourceSizeMode sizeMode = EffectResourceSizeMode::MatchViewport) const;
    void CreateEffectResource(EffectResource& resource,
                              std::uint32_t width, std::uint32_t height,
                              EffectResourceSizeMode sizeMode = EffectResourceSizeMode::MatchViewport) const;
    [[nodiscard]] EffectCubeMapResource CreateCubeMapResource(
        const std::filesystem::path& path, bool createRenderTargetView = false,
        bool createFaceRenderTargetViews = false, bool forceSrgb = true) const;

    [[nodiscard]] ID3D11DepthStencilState* GetDepthStencilState(DepthMode mode) const noexcept;
    [[nodiscard]] ID3D11BlendState* GetBlendState(BlendMode mode) const noexcept;
    [[nodiscard]] ID3D11RasterizerState* GetRasterizerState(RasterizerMode mode) const noexcept;
    [[nodiscard]] ID3D11SamplerState* GetLinearClampSampler() const noexcept;

    void SetDepthMode(DepthMode mode);
    void SetStencil(const StencilDescription& description);
    void SetBlendMode(BlendMode mode, const std::array<float, 4>& blendFactor = {}, UINT sampleMask = 0xffffffffU);
    void SetBlendState(const D3D11_BLEND_DESC& description, const std::array<float, 4>& blendFactor = {}, UINT sampleMask = 0xffffffffU);
    void SetRasterizerMode(RasterizerMode mode);
    void SetRasterizerState(const D3D11_RASTERIZER_DESC& description);

    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11DepthStencilState> CreateDepthStencilState(
        const D3D11_DEPTH_STENCIL_DESC& description) const;
    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11BlendState> CreateBlendState(
        const D3D11_BLEND_DESC& description) const;
    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11RasterizerState> CreateRasterizerState(
        const D3D11_RASTERIZER_DESC& description) const;

  private:
    EffectManager(ID3D11Device* device, ID3D11DeviceContext* context);
    void CreateCommonStates();
    [[nodiscard]] static D3D11_DEPTH_STENCIL_DESC BuildDepthStencilDescription(DepthMode mode);
    [[nodiscard]] static D3D11_BLEND_DESC BuildBlendDescription(BlendMode mode);
    [[nodiscard]] static D3D11_RASTERIZER_DESC BuildRasterizerDescription(RasterizerMode mode);

    static std::unique_ptr<EffectManager> m_instance;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    std::unique_ptr<DirectX::CommonStates> m_commonStates;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthReadOnlyState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthReadWriteState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_opaqueBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_additiveBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_premultipliedBlendState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_solidCullClockwiseState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_solidCullNoneState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframeCullClockwiseState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframeCullNoneState;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_solidCullClockwiseScissorState;
};

} // namespace lrender
