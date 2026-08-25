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
#include "render/Mesh.h"
#include "render/ResourceCache.h"
#include "render/RenderTarget.h"

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
    [[nodiscard]] ID3D11ShaderResourceView* MaterialPreview(const Entity& entity);

    [[nodiscard]] ID3D11Device* Device() const noexcept { return device_.Get(); }
    [[nodiscard]] ID3D11DeviceContext* Context() const noexcept { return context_.Get(); }
    [[nodiscard]] RenderTarget& ViewportTarget() noexcept { return viewportTarget_; }
    [[nodiscard]] BasicMeshEffect& Effect() noexcept { return *effect_; }
    [[nodiscard]] std::size_t CachedMeshAssetCount() const noexcept;
    [[nodiscard]] std::size_t CachedTextureCount() const noexcept;
    [[nodiscard]] HWND WindowHandle() const noexcept { return windowHandle_; }

private:
    void CreateBackBuffer();
    [[nodiscard]] Material ResolveMaterial(
        const Material& source, const EntityMaterial& settings);

    HWND windowHandle_{};
    std::uint32_t swapChainWidth_{};
    std::uint32_t swapChainHeight_{};
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> backBufferView_;
    RenderTarget viewportTarget_;
    std::unique_ptr<Mesh> cubeMesh_;
    std::unique_ptr<Mesh> sphereMesh_;
    std::unique_ptr<Mesh> planeMesh_;
    std::unique_ptr<BasicMeshEffect> effect_;
    std::unique_ptr<ResourceCache> resources_;
    Material primitiveMaterial_;
};

} // namespace lrender
