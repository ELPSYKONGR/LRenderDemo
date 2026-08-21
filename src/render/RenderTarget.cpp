/**
 * @file Offscreen D3D11 render target implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends render/RenderTarget.h
 */
#include "render/RenderTarget.h"

#include <algorithm>
#include <stdexcept>

namespace lrender {

void RenderTarget::Reset() noexcept {
    depthStencilView_.Reset();
    depthTexture_.Reset();
    shaderResourceView_.Reset();
    renderTargetView_.Reset();
    colorTexture_.Reset();
    width_ = 0;
    height_ = 0;
}

void RenderTarget::Resize(ID3D11Device* device, std::uint32_t width, std::uint32_t height) {
    if (device == nullptr) {
        throw std::invalid_argument("RenderTarget resize requires a D3D11 device");
    }
    width = std::max(width, 1U);
    height = std::max(height, 1U);
    if (width == width_ && height == height_) {
        return;
    }

    D3D11_TEXTURE2D_DESC colorDescription{};
    colorDescription.Width = width;
    colorDescription.Height = height;
    colorDescription.MipLevels = 1;
    colorDescription.ArraySize = 1;
    colorDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    colorDescription.SampleDesc.Count = 1;
    colorDescription.Usage = D3D11_USAGE_DEFAULT;
    colorDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> colorTexture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    if (FAILED(device->CreateTexture2D(&colorDescription, nullptr, colorTexture.GetAddressOf())) ||
        FAILED(device->CreateRenderTargetView(colorTexture.Get(), nullptr, renderTargetView.GetAddressOf())) ||
        FAILED(device->CreateShaderResourceView(colorTexture.Get(), nullptr, shaderResourceView.GetAddressOf()))) {
        throw std::runtime_error("Failed to create viewport color target");
    }

    D3D11_TEXTURE2D_DESC depthDescription = colorDescription;
    depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    if (FAILED(device->CreateTexture2D(&depthDescription, nullptr, depthTexture.GetAddressOf())) ||
        FAILED(device->CreateDepthStencilView(depthTexture.Get(), nullptr, depthStencilView.GetAddressOf()))) {
        throw std::runtime_error("Failed to create viewport depth target");
    }

    width_ = width;
    height_ = height;
    colorTexture_ = std::move(colorTexture);
    renderTargetView_ = std::move(renderTargetView);
    shaderResourceView_ = std::move(shaderResourceView);
    depthTexture_ = std::move(depthTexture);
    depthStencilView_ = std::move(depthStencilView);
}

void RenderTarget::BindAndClear(ID3D11DeviceContext* context, const float clearColor[4]) const {
    if (context == nullptr || renderTargetView_ == nullptr || depthStencilView_ == nullptr) {
        throw std::runtime_error("RenderTarget is not ready");
    }
    ID3D11RenderTargetView* target = renderTargetView_.Get();
    context->OMSetRenderTargets(1, &target, depthStencilView_.Get());
    context->ClearRenderTargetView(target, clearColor);
    context->ClearDepthStencilView(
        depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MinDepth = 0.0F;
    viewport.MaxDepth = 1.0F;
    context->RSSetViewports(1, &viewport);
}

} // namespace lrender
