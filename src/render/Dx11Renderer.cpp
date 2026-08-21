/**
 * @file D3D11 renderer implementation and frame orchestration.
 * @author Codex
 * @created 2026-08-20
 * @depends render/Dx11Renderer.h, render/PrimitiveFactory.h, ImGui DX11 backend
 */
#include "render/Dx11Renderer.h"

#include "render/PrimitiveFactory.h"

#include <backends/imgui_impl_dx11.h>
#include <algorithm>
#include <array>
#include <stdexcept>

namespace lrender {

void Dx11Renderer::Initialize(HWND windowHandle, std::uint32_t width, std::uint32_t height) {
    if (windowHandle == nullptr) {
        throw std::invalid_argument("Renderer requires a valid Win32 window");
    }
    windowHandle_ = windowHandle;
    swapChainWidth_ = width;
    swapChainHeight_ = height;

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
        swapChain_.ReleaseAndGetAddressOf(),
        device_.ReleaseAndGetAddressOf(),
        &createdFeatureLevel,
        context_.ReleaseAndGetAddressOf());
#if defined(_DEBUG)
    if (FAILED(result)) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels.data(),
            static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION,
            &swapChainDescription, swapChain_.ReleaseAndGetAddressOf(),
            device_.ReleaseAndGetAddressOf(), &createdFeatureLevel,
            context_.ReleaseAndGetAddressOf());
    }
#endif
    if (FAILED(result)) {
        throw std::runtime_error("D3D11CreateDeviceAndSwapChain failed");
    }

    CreateBackBuffer();
    viewportTarget_.Resize(device_.Get(), 960, 640);
    cubeMesh_ = PrimitiveFactory::CreateCube(device_.Get());
    sphereMesh_ = PrimitiveFactory::CreateSphere(device_.Get());
    effect_ = std::make_unique<BasicMeshEffect>(device_.Get(), LRENDER_SHADER_OUTPUT_DIR);
    resources_ = std::make_unique<ResourceCache>(device_.Get(), context_.Get());
    primitiveMaterial_ = resources_->CheckerMaterial();
}

void Dx11Renderer::Shutdown() noexcept {
    if (context_) {
        context_->ClearState();
        context_->Flush();
    }
    effect_.reset();
    primitiveMaterial_ = {};
    resources_.reset();
    sphereMesh_.reset();
    cubeMesh_.reset();
    viewportTarget_.Reset();
    backBufferView_.Reset();
    swapChain_.Reset();
    context_.Reset();
    device_.Reset();
}

void Dx11Renderer::CreateBackBuffer() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()))) ||
        FAILED(device_->CreateRenderTargetView(
            backBuffer.Get(), nullptr, backBufferView_.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error("Failed to create swap-chain back buffer");
    }
}

void Dx11Renderer::ResizeSwapChain(std::uint32_t width, std::uint32_t height) {
    width = std::max(width, 1U);
    height = std::max(height, 1U);
    if (width == swapChainWidth_ && height == swapChainHeight_) {
        return;
    }
    backBufferView_.Reset();
    if (FAILED(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0))) {
        throw std::runtime_error("Failed to resize swap chain");
    }
    swapChainWidth_ = width;
    swapChainHeight_ = height;
    CreateBackBuffer();
}

void Dx11Renderer::ResizeViewport(std::uint32_t width, std::uint32_t height) {
    ID3D11ShaderResourceView* nullResource = nullptr;
    context_->PSSetShaderResources(0, 1, &nullResource);
    viewportTarget_.Resize(device_.Get(), width, height);
}

void Dx11Renderer::RenderScene(
    const Scene& scene, const Camera& camera, std::uint32_t selectedEntityId) {
    // ImGui sampled this texture in the previous frame; unbind it before using the same resource as an RTV.
    ID3D11ShaderResourceView* nullResource = nullptr;
    context_->PSSetShaderResources(0, 1, &nullResource);
    constexpr float clearColor[4]{0.055F, 0.065F, 0.075F, 1.0F};
    viewportTarget_.BindAndClear(context_.Get(), clearColor);
    const float aspect = static_cast<float>(viewportTarget_.Width()) /
                         static_cast<float>(viewportTarget_.Height());
    const auto view = camera.ViewMatrix();
    const auto projection = camera.ProjectionMatrix(aspect);
    const auto cameraPosition = camera.Position();

    for (const Entity& entity : scene.Entities()) {
        const auto world = entity.transform.ToMatrix();
        const bool isSelected = entity.id == selectedEntityId;
        if (entity.IsModel()) {
            const std::shared_ptr<Model> model = resources_->LoadModel(entity.modelPath);
            for (const ModelPart& part : model->parts) {
                effect_->Bind(
                    context_.Get(), world, view, projection, cameraPosition,
                    part.material, entity.color, isSelected);
                part.mesh->Draw(context_.Get());
            }
        } else {
            effect_->Bind(
                context_.Get(), world, view, projection, cameraPosition,
                primitiveMaterial_, entity.color, isSelected);
            const Mesh& mesh = entity.primitive == PrimitiveType::Cube ? *cubeMesh_ : *sphereMesh_;
            mesh.Draw(context_.Get());
        }
    }
    nullResource = nullptr;
    ID3D11SamplerState* nullSampler = nullptr;
    context_->PSSetShaderResources(0, 1, &nullResource);
    context_->PSSetSamplers(0, 1, &nullSampler);
    context_->RSSetState(nullptr);
}

void Dx11Renderer::PreloadModel(const std::filesystem::path& path) {
    if (!resources_) {
        throw std::logic_error("Renderer resource cache is not initialized");
    }
    static_cast<void>(resources_->LoadModel(path));
}

std::size_t Dx11Renderer::CachedModelCount() const noexcept {
    return resources_ ? resources_->ModelCount() : 0;
}

std::size_t Dx11Renderer::CachedTextureCount() const noexcept {
    return resources_ ? resources_->TextureCount() : 0;
}

void Dx11Renderer::RenderEditor(ImDrawData* drawData) {
    constexpr float background[4]{0.10F, 0.105F, 0.115F, 1.0F};
    ID3D11RenderTargetView* target = backBufferView_.Get();
    context_->OMSetRenderTargets(1, &target, nullptr);
    context_->ClearRenderTargetView(target, background);
    ImGui_ImplDX11_RenderDrawData(drawData);
}

void Dx11Renderer::Present() {
    const HRESULT result = swapChain_->Present(1, 0);
    if (FAILED(result)) {
        throw std::runtime_error("Swap-chain presentation failed");
    }
}

} // namespace lrender
