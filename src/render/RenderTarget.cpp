/**
 * @file Offscreen D3D11 render target implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends render/RenderTarget.h
 */
#include "render/RenderTarget.h"

#include <algorithm>
#include <stdexcept>

namespace lrender
{

void RenderTarget::Reset() noexcept
{
    m_depthStencilView.Reset();
    m_depthTexture.Reset();
    m_shaderResourceView.Reset();
    m_renderTargetView.Reset();
    m_colorTexture.Reset();
    m_width = 0;
    m_height = 0;
}

void RenderTarget::Resize(ID3D11Device* device, std::uint32_t width, std::uint32_t height)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("RenderTarget resize requires a D3D11 device");
    }
    width = std::max(width, 1U);
    height = std::max(height, 1U);
    if (width == m_width && height == m_height)
    {
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
        FAILED(device->CreateShaderResourceView(colorTexture.Get(), nullptr, shaderResourceView.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create viewport color target");
    }

    D3D11_TEXTURE2D_DESC depthDescription = colorDescription;
    depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
    if (FAILED(device->CreateTexture2D(&depthDescription, nullptr, depthTexture.GetAddressOf())) ||
        FAILED(device->CreateDepthStencilView(depthTexture.Get(), nullptr, depthStencilView.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create viewport depth target");
    }

    m_width = width;
    m_height = height;
    m_colorTexture = std::move(colorTexture);
    m_renderTargetView = std::move(renderTargetView);
    m_shaderResourceView = std::move(shaderResourceView);
    m_depthTexture = std::move(depthTexture);
    m_depthStencilView = std::move(depthStencilView);
}

void RenderTarget::BindAndClear(ID3D11DeviceContext* context, const float clearColor[4],
                                const RenderTarget* additionalTarget) const
{
    if (context == nullptr || m_renderTargetView == nullptr || m_depthStencilView == nullptr)
    {
        throw std::runtime_error("RenderTarget is not ready");
    }
    UINT targetCount = 1;
    ID3D11RenderTargetView* targets[2] = {m_renderTargetView.Get(), nullptr};
    if (additionalTarget != nullptr)
    {
        if (additionalTarget->m_renderTargetView == nullptr || additionalTarget->m_width != m_width ||
            additionalTarget->m_height != m_height)
        {
            throw std::invalid_argument("Additional render target is incompatible");
        }
        targets[1] = additionalTarget->m_renderTargetView.Get();
        targetCount = 2;
    }
    context->OMSetRenderTargets(targetCount, targets, m_depthStencilView.Get());
    for (UINT index = 0; index < targetCount; ++index)
    {
        context->ClearRenderTargetView(targets[index], clearColor);
    }
    context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0F;
    viewport.MaxDepth = 1.0F;
    context->RSSetViewports(1, &viewport);
}

ID3D11ShaderResourceView* RenderTarget::GetShaderResourceView() const noexcept
{
    return m_shaderResourceView.Get();
}

ID3D11RenderTargetView* RenderTarget::GetRenderTargetView() const noexcept
{
    return m_renderTargetView.Get();
}

std::uint32_t RenderTarget::GetWidth() const noexcept
{
    return m_width;
}

std::uint32_t RenderTarget::GetHeight() const noexcept
{
    return m_height;
}

} // namespace lrender
