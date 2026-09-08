/**
 * @file View management and DX11 pipeline state protection.
 */
#pragma once

#include "core/Camera.h"
#include "render/EffectResource.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <d3d11.h>
#include <memory>
#include <unordered_map>
#include <wrl/client.h>
#include <windows.h>

namespace lrender
{

class ViewStateGuard final
{
  public:
    explicit ViewStateGuard(ID3D11DeviceContext* context);
    ~ViewStateGuard();

    ViewStateGuard(const ViewStateGuard&) = delete;
    ViewStateGuard& operator=(const ViewStateGuard&) = delete;

  private:
    ID3D11DeviceContext* m_context = nullptr;
    std::array<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>, 8> m_renderTargets;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11HullShader> m_hullShader;
    Microsoft::WRL::ComPtr<ID3D11DomainShader> m_domainShader;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
    std::array<float, 4> m_blendFactor = {};
    UINT m_sampleMask = 0xffffffffU;
    UINT m_stencilReference = 0;
    D3D11_PRIMITIVE_TOPOLOGY m_primitiveTopology = {};
    std::array<D3D11_VIEWPORT, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE> m_viewports = {};
    UINT m_viewportCount = 0;
};

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

using ViewId = std::uint32_t;

struct ViewInfo
{
    ViewId id = 0;
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    HWND windowHandle = nullptr;
    Camera camera;
    std::unique_ptr<EffectResource> resource;
};

class ViewManager final
{
  public:
    static void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    static void Shutdown() noexcept;
    [[nodiscard]] static ViewManager& Instance();

    ViewManager(const ViewManager&) = delete;
    ViewManager& operator=(const ViewManager&) = delete;

    [[nodiscard]] ViewId CreateView(std::uint32_t width, std::uint32_t height, HWND windowHandle = nullptr);
    bool RemoveView(ViewId id) noexcept;
    [[nodiscard]] ViewInfo* FindView(ViewId id) noexcept;
    [[nodiscard]] const ViewInfo* FindView(ViewId id) const noexcept;
    void ResizeView(ViewId id, std::uint32_t width, std::uint32_t height);
    void AttachWindow(ViewId id, HWND windowHandle);

    void SetActiveView(ViewId id);
    [[nodiscard]] ViewId ActiveViewId() const noexcept;
    [[nodiscard]] ViewInfo* ActiveView() noexcept;
    [[nodiscard]] std::size_t ViewCount() const noexcept;

    [[nodiscard]] std::unique_ptr<ViewStateGuard> CaptureState() const;
    [[nodiscard]] ID3D11DepthStencilState* GetDepthStencilState(DepthMode mode) const noexcept;
    [[nodiscard]] ID3D11BlendState* GetBlendState(BlendMode mode) const noexcept;
    void SetDepthMode(DepthMode mode);
    void SetStencil(const StencilDescription& description);
    void SetBlendMode(BlendMode mode, const std::array<float, 4>& blendFactor = {},
                      UINT sampleMask = 0xffffffffU);
    void SetBlendState(const D3D11_BLEND_DESC& description, const std::array<float, 4>& blendFactor = {},
                       UINT sampleMask = 0xffffffffU);
    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11DepthStencilState> CreateDepthStencilState(
        const D3D11_DEPTH_STENCIL_DESC& description) const;
    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D11BlendState> CreateBlendState(
        const D3D11_BLEND_DESC& description) const;

  private:
    ViewManager(ID3D11Device* device, ID3D11DeviceContext* context);
    void CreateCommonStates();
    [[nodiscard]] static D3D11_DEPTH_STENCIL_DESC BuildDepthStencilDescription(DepthMode mode);
    [[nodiscard]] static D3D11_BLEND_DESC BuildBlendDescription(BlendMode mode);

    static std::unique_ptr<ViewManager> m_instance;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthReadOnlyState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthReadWriteState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_opaqueBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_additiveBlendState;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_premultipliedBlendState;
    std::unordered_map<ViewId, ViewInfo> m_views;
    ViewId m_nextViewId = 1;
    ViewId m_activeViewId = 0;
};

} // namespace lrender
