/**
 * @file D3D11 device, swap chain, viewport, mesh, and Effect coordinator.
 * @author Codex
 * @created 2026-08-20
 * @depends render/RenderTarget.h, render/BasicMeshEffect.h, core/Scene.h
 */
#pragma once

#include "core/Camera.h"
#include "core/Scene.h"
#include "render/BasicMeshEffect.h"
#include "render/ColorProcessorEffect.h"
#include "render/Mesh.h"
#include "render/ResourceCache.h"
#include "render/RenderTarget.h"
#include "render/SolidMeshCache.h"

#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <windows.h>
#include <wrl/client.h>

struct ImDrawData;

namespace lrender {

class Dx11Renderer final {
public:
    void Initialize(HWND windowHandle, std::uint32_t width, std::uint32_t height);
    void Shutdown() noexcept;
    void ResizeSwapChain(std::uint32_t width, std::uint32_t height);
    void ResizeViewport(std::uint32_t width, std::uint32_t height);
    void RenderScene(const Scene& scene, const Camera& camera, std::uint32_t selectedEntityId);
    void RenderEditor(ImDrawData* drawData);
    void Present();
    [[nodiscard]] std::shared_ptr<const MeshAsset> PreloadModel(
        const std::filesystem::path& path);
    void PreloadTexture(const std::filesystem::path& path);
    void ClearRuntimeCaches() noexcept;
    [[nodiscard]] ID3D11ShaderResourceView* MaterialPreview(const Entity& entity);

    [[nodiscard]] ID3D11Device* Device() const noexcept { return m_device.Get(); }
    [[nodiscard]] ID3D11DeviceContext* Context() const noexcept { return m_context.Get(); }
    [[nodiscard]] RenderTarget& ViewportTarget() noexcept { return m_viewportTarget; }
    [[nodiscard]] const RenderTarget& NormalTarget() const noexcept { return m_normalTarget; }
    [[nodiscard]] BasicMeshEffect& Effect() noexcept { return *m_effect; }
    [[nodiscard]] std::size_t CachedMeshAssetCount() const noexcept;
    [[nodiscard]] std::size_t CachedTextureCount() const noexcept;
    [[nodiscard]] HWND WindowHandle() const noexcept { return m_windowHandle; }

private:
    void CreateBackBuffer();
    [[nodiscard]] Material ResolveMaterial(
        const Material& source, const EntityMaterial& settings);

    HWND m_windowHandle = nullptr;
    std::uint32_t m_swapChainWidth = 0;
    std::uint32_t m_swapChainHeight = 0;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferView;
    RenderTarget m_sceneTarget;
    RenderTarget m_normalTarget;
    RenderTarget m_viewportTarget;
    std::unique_ptr<SolidMeshCache> m_solidMeshes;
    std::unique_ptr<BasicMeshEffect> m_effect;
    std::unique_ptr<ColorProcessorEffect> m_colorProcessor;
    std::unique_ptr<ResourceCache> m_resources;
};

} // namespace lrender
