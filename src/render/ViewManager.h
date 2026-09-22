/**
 * @file View management and DX11 pipeline state snapshot protection.
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

class Scene;

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
    [[nodiscard]] bool FitView(Camera& camera, const BoundingBox& boundingBox, float aspectRatio,
                               float margin = 1.15F) const noexcept;
    void CreateSphereTestEntity(Scene& scene, std::uint32_t modelId) const;
    void CreatePlaneTestEntity(Scene& scene, std::uint32_t modelId) const;
    void CreateCornellBoxTestScene(Scene& scene, std::uint32_t modelId) const;

  private:
    ViewManager(ID3D11Device* device, ID3D11DeviceContext* context);

    static std::unique_ptr<ViewManager> m_instance;
    ID3D11DeviceContext* m_context = nullptr;
    std::unordered_map<ViewId, ViewInfo> m_views;
    ViewId m_nextViewId = 1;
    ViewId m_activeViewId = 0;
};

} // namespace lrender
