/**
 * @file D3D11 renderer implementation and frame orchestration.
 * @author Codex
 * @created 2026-08-20
 * @depends render/Dx11Renderer.h, render/PrimitiveFactory.h, ImGui DX11 backend
 */
#include "render/Dx11Renderer.h"
#include "render/ViewManager.h"

#include <backends/imgui_impl_dx11.h>
#include <algorithm>
#include <array>
#include <stdexcept>
#include <unordered_set>

namespace lrender {
namespace {

D3D11_FILTER NativeFilter(MaterialFilter filter) {
    switch (filter) {
    case MaterialFilter::Point: return D3D11_FILTER_MIN_MAG_MIP_POINT;
    case MaterialFilter::Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    case MaterialFilter::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
    }
    throw std::invalid_argument("Unsupported material texture filter");
}

D3D11_TEXTURE_ADDRESS_MODE NativeAddressMode(MaterialAddressMode mode) {
    switch (mode) {
    case MaterialAddressMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
    case MaterialAddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
    case MaterialAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
    }
    throw std::invalid_argument("Unsupported material texture address mode");
}

} // namespace

void Dx11Renderer::Initialize(HWND windowHandle, std::uint32_t width, std::uint32_t height) {
    if (windowHandle == nullptr) {
        throw std::invalid_argument("Renderer requires a valid Win32 window");
    }
    m_windowHandle = windowHandle;
    m_swapChainWidth = width;
    m_swapChainHeight = height;

    DXGI_SWAP_CHAIN_DESC swapChainDescription{};
    swapChainDescription.BufferCount = 2;
    swapChainDescription.BufferDesc.Width = width;
    swapChainDescription.BufferDesc.Height = height;
    swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.OutputWindow = windowHandle;
    swapChainDescription.SampleDesc.Count = 1;
    swapChainDescription.Windowed = TRUE;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    constexpr std::array featureLevels{D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL createdFeatureLevel{};
    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels.data(),
        static_cast<UINT>(featureLevels.size()),
        D3D11_SDK_VERSION,
        &swapChainDescription,
        m_swapChain.ReleaseAndGetAddressOf(),
        m_device.ReleaseAndGetAddressOf(),
        &createdFeatureLevel,
        m_context.ReleaseAndGetAddressOf());
#if defined(_DEBUG)
    if (FAILED(result)) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels.data(),
            static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION,
            &swapChainDescription, m_swapChain.ReleaseAndGetAddressOf(),
            m_device.ReleaseAndGetAddressOf(), &createdFeatureLevel,
            m_context.ReleaseAndGetAddressOf());
    }
#endif
    if (FAILED(result)) {
        throw std::runtime_error("D3D11CreateDeviceAndSwapChain failed");
    }

    CreateBackBuffer();
    ViewManager::Initialize(m_device.Get(), m_context.Get());
    m_sceneTarget.Resize(m_device.Get(), 960, 640);
    m_normalTarget.Resize(m_device.Get(), 960, 640);
    m_viewportTarget.Resize(m_device.Get(), 960, 640);
    m_solidMeshes = std::make_unique<SolidMeshCache>(m_device.Get());
    m_effect = std::make_unique<BasicMeshEffect>(
        m_device.Get(), m_context.Get(), LRENDER_SHADER_OUTPUT_DIR);
    m_colorProcessor = std::make_unique<ColorProcessorEffect>(
        m_device.Get(), m_context.Get(), LRENDER_SHADER_OUTPUT_DIR);
    m_resources = std::make_unique<ResourceCache>(m_device.Get(), m_context.Get());
}

void Dx11Renderer::Shutdown() noexcept {
    ViewManager::Shutdown();
    if (m_context) {
        m_context->ClearState();
        m_context->Flush();
    }
    m_effect.reset();
    m_colorProcessor.reset();
    m_resources.reset();
    m_solidMeshes.reset();
    m_viewportTarget.Reset();
    m_normalTarget.Reset();
    m_sceneTarget.Reset();
    m_backBufferView.Reset();
    m_swapChain.Reset();
    m_context.Reset();
    m_device.Reset();
}

void Dx11Renderer::CreateBackBuffer() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))) ||
        FAILED(m_device->CreateRenderTargetView(
            backBuffer.Get(), nullptr, m_backBufferView.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error("Failed to create swap-chain back buffer");
    }
}

void Dx11Renderer::ResizeSwapChain(std::uint32_t width, std::uint32_t height) {
    width = std::max(width, 1U);
    height = std::max(height, 1U);
    if (width == m_swapChainWidth && height == m_swapChainHeight) {
        return;
    }
    m_backBufferView.Reset();
    if (FAILED(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0))) {
        throw std::runtime_error("Failed to resize swap chain");
    }
    m_swapChainWidth = width;
    m_swapChainHeight = height;
    CreateBackBuffer();
}

void Dx11Renderer::ResizeViewport(std::uint32_t width, std::uint32_t height) {
    ID3D11ShaderResourceView* nullResource = nullptr;
    m_context->PSSetShaderResources(0, 1, &nullResource);
    m_sceneTarget.Resize(m_device.Get(), width, height);
    m_normalTarget.Resize(m_device.Get(), width, height);
    m_viewportTarget.Resize(m_device.Get(), width, height);
}

void Dx11Renderer::RenderScene(
    const Scene& scene, const Camera& camera, std::uint32_t selectedEntityId) {
    // ImGui sampled this texture in the previous frame; unbind it before using the same resource as an RTV.
    ID3D11ShaderResourceView* nullResource = nullptr;
    m_context->PSSetShaderResources(0, 1, &nullResource);
    constexpr float clearColor[4] = {0.055F, 0.065F, 0.075F, 1.0F};
    m_sceneTarget.BindAndClear(m_context.Get(), clearColor, &m_normalTarget);
    constexpr float normalClearColor[4] = {0.5F, 0.5F, 0.5F, 1.0F};
    m_context->ClearRenderTargetView(
        m_normalTarget.GetRenderTargetView(), normalClearColor);
    const float aspect = static_cast<float>(m_sceneTarget.GetWidth()) /
                         static_cast<float>(m_sceneTarget.GetHeight());
    const EffectFrameContext frameContext(m_context.Get(), camera, aspect);
    std::unordered_set<EntityId> activeSolids;

    for (const Model& sceneModel : scene.Models()) {
        for (const Entity& entity : sceneModel.entities) {
            if (const MeshGeometry* meshGeometry = entity.Mesh()) {
                const auto asset = m_resources->LoadMeshAsset(meshGeometry->assetPath);
                if (meshGeometry->assetEntityIndex >= asset->entities.size()) {
                    throw std::runtime_error("Mesh entity index is outside the cached asset");
                }
                for (const MeshPart& part :
                     asset->entities[meshGeometry->assetEntityIndex].parts) {
                    const EffectDrawContext drawContext(
                        entity, ResolveMaterial(part.material, entity.EffectiveMaterial()),
                        selectedEntityId, part.mesh.get());
                    m_effect->Draw(frameContext, drawContext);
                }
                continue;
            }

            const SolidGeometry* solid = entity.Solid();
            if (solid == nullptr || m_solidMeshes == nullptr) {
                throw std::runtime_error("Solid geometry cache is not initialized");
            }
            activeSolids.insert(entity.id);
            const Mesh& mesh = m_solidMeshes->Resolve(entity.id, *solid);
            const EffectDrawContext drawContext(
                entity, ResolveMaterial(m_resources->DefaultMaterial(), entity.EffectiveMaterial()),
                selectedEntityId, &mesh);
            m_effect->Draw(frameContext, drawContext);
        }
    }
    m_solidMeshes->Prune(activeSolids);
    nullResource = nullptr;
    ID3D11SamplerState* nullSampler = nullptr;
    m_context->PSSetShaderResources(0, 1, &nullResource);
    m_context->PSSetSamplers(0, 1, &nullSampler);
    m_context->RSSetState(nullptr);

    m_viewportTarget.BindAndClear(m_context.Get(), clearColor);
    m_colorProcessor->Draw(m_context.Get(), m_sceneTarget.GetShaderResourceView());
}

std::shared_ptr<const MeshAsset> Dx11Renderer::PreloadModel(
    const std::filesystem::path& path) {
    if (!m_resources) {
        throw std::logic_error("Renderer resource cache is not initialized");
    }
    return m_resources->LoadMeshAsset(path);
}

void Dx11Renderer::PreloadTexture(const std::filesystem::path& path) {
    if (!m_resources) {
        throw std::logic_error("Renderer resource cache is not initialized");
    }
    static_cast<void>(m_resources->LoadTexture(path));
}

void Dx11Renderer::ClearRuntimeCaches() noexcept {
    if (m_solidMeshes) {
        m_solidMeshes->Clear();
    }
}

ID3D11ShaderResourceView* Dx11Renderer::MaterialPreview(const Entity& entity) {
    if (!m_resources) {
        throw std::logic_error("Renderer resource cache is not initialized");
    }
    const Material* source = nullptr;
    std::shared_ptr<MeshAsset> asset;
    if (const MeshGeometry* meshGeometry = entity.Mesh()) {
        asset = m_resources->LoadMeshAsset(meshGeometry->assetPath);
        if (meshGeometry->assetEntityIndex >= asset->entities.size() ||
            asset->entities[meshGeometry->assetEntityIndex].parts.empty()) {
            return nullptr;
        }
        source = &asset->entities[meshGeometry->assetEntityIndex].parts.front().material;
    }
    Material defaultMaterial;
    if (source == nullptr) {
        defaultMaterial = m_resources->DefaultMaterial();
        source = &defaultMaterial;
    }
    const Material resolved = ResolveMaterial(*source, entity.EffectiveMaterial());
    return resolved.baseColorTexture ? resolved.baseColorTexture->ShaderResourceView() : nullptr;
}

Material Dx11Renderer::ResolveMaterial(
    const Material& source, const EntityMaterial& settings) {
    Material resolved = source;
    resolved.diffuseStrength = settings.diffuseStrength;
    resolved.specularColor = settings.specularColor;
    resolved.specularStrength = settings.specularStrength;
    resolved.shininess = settings.shininess;
    resolved.doubleSided = settings.doubleSided;
    resolved.displayMode = settings.displayMode;

    SamplerDescription samplerDescription;
    samplerDescription.filter = NativeFilter(settings.filter);
    samplerDescription.addressU = NativeAddressMode(settings.addressMode);
    samplerDescription.addressV = samplerDescription.addressU;
    resolved.sampler = m_resources->GetSampler(samplerDescription);

    if (settings.displayMode == SurfaceDisplayMode::LitUntextured) {
        resolved.baseColorTexture = m_resources->DefaultMaterial().baseColorTexture;
    } else if (!settings.useSourceTexture && !settings.baseColorTexturePath.empty()) {
        resolved.baseColorTexture = m_resources->LoadTexture(settings.baseColorTexturePath);
    }
    return resolved;
}

std::size_t Dx11Renderer::CachedMeshAssetCount() const noexcept {
    return m_resources ? m_resources->MeshAssetCount() : 0;
}

std::size_t Dx11Renderer::CachedTextureCount() const noexcept {
    return m_resources ? m_resources->TextureCount() : 0;
}

void Dx11Renderer::RenderEditor(ImDrawData* drawData) {
    constexpr float background[4]{0.10F, 0.105F, 0.115F, 1.0F};
    ID3D11RenderTargetView* target = m_backBufferView.Get();
    m_context->OMSetRenderTargets(1, &target, nullptr);
    m_context->ClearRenderTargetView(target, background);
    ImGui_ImplDX11_RenderDrawData(drawData);
}

void Dx11Renderer::Present() {
    const HRESULT result = m_swapChain->Present(1, 0);
    if (FAILED(result)) {
        throw std::runtime_error("Swap-chain presentation failed");
    }
}

} // namespace lrender
