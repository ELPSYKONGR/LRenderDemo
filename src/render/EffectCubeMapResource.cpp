/**
 * @file Effect-owned TextureCube resource implementation.
 * @author Codex
 * @created 2026-09-07
 * @depends render/EffectCubeMapResource.h, DirectXTK DDSTextureLoader
 */
#include "render/EffectCubeMapResource.h"

#include <DDSTextureLoader.h>
#include <algorithm>
#include <stdexcept>
#include <string>

namespace lrender
{
namespace
{

std::string PathForMessage(const std::filesystem::path& path)
{
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

void ValidateDevice(ID3D11Device* device)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("EffectCubeMapResource requires a D3D11 device");
    }
}

} // namespace

void EffectCubeMapResource::LoadDDS(ID3D11Device* device, const std::filesystem::path& path,
                                    bool createRenderTargetView, bool createFaceRenderTargetViews, bool forceSrgb)
{
    ValidateDevice(device);
    if (!std::filesystem::is_regular_file(path))
    {
        throw std::runtime_error("Cubemap DDS file does not exist: " + PathForMessage(path));
    }

    const bool needsRenderTarget = createRenderTargetView || createFaceRenderTargetViews;
    const unsigned int bindFlags = D3D11_BIND_SHADER_RESOURCE | (needsRenderTarget ? D3D11_BIND_RENDER_TARGET : 0U);
    const auto loadFlags = forceSrgb ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_DEFAULT;

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    const HRESULT result = DirectX::CreateDDSTextureFromFileEx(
        device, path.c_str(), 0, D3D11_USAGE_DEFAULT, bindFlags, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, loadFlags,
        resource.GetAddressOf(), shaderResourceView.GetAddressOf());
    if (FAILED(result))
    {
        throw std::runtime_error("Failed to load cubemap DDS: " + PathForMessage(path));
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(resource.As(&texture)))
    {
        throw std::runtime_error("Cubemap DDS is not a Texture2D array: " + PathForMessage(path));
    }

    D3D11_TEXTURE2D_DESC description{};
    texture->GetDesc(&description);
    if ((description.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) == 0 || description.ArraySize != 6 ||
        description.Width != description.Height)
    {
        throw std::runtime_error("DDS is not a single square cubemap: " + PathForMessage(path));
    }

    Reset();
    m_size = description.Width;
    m_texture = std::move(texture);
    m_shaderResourceView = std::move(shaderResourceView);
    CreateRenderTargetViews(device, description.Format, createRenderTargetView, createFaceRenderTargetViews);
}

void EffectCubeMapResource::Create(ID3D11Device* device, std::uint32_t size, DXGI_FORMAT format,
                                   bool createRenderTargetView, bool createFaceRenderTargetViews)
{
    ValidateDevice(device);
    size = std::max(size, 1U);
    const bool needsRenderTarget = createRenderTargetView || createFaceRenderTargetViews;

    D3D11_TEXTURE2D_DESC description{};
    description.Width = size;
    description.Height = size;
    description.MipLevels = 1;
    description.ArraySize = 6;
    description.Format = format;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_DEFAULT;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE | (needsRenderTarget ? D3D11_BIND_RENDER_TARGET : 0U);
    description.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&description, nullptr, texture.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create effect cubemap texture");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDescription{};
    shaderResourceDescription.Format = format;
    shaderResourceDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    shaderResourceDescription.TextureCube.MostDetailedMip = 0;
    shaderResourceDescription.TextureCube.MipLevels = 1;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
    if (FAILED(device->CreateShaderResourceView(texture.Get(), &shaderResourceDescription,
                                                shaderResourceView.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create effect cubemap shader-resource view");
    }

    Reset();
    m_size = size;
    m_texture = std::move(texture);
    m_shaderResourceView = std::move(shaderResourceView);
    CreateRenderTargetViews(device, format, createRenderTargetView, createFaceRenderTargetViews);
}

void EffectCubeMapResource::Reset() noexcept
{
    for (auto& faceRenderTargetView : m_faceRenderTargetViews)
    {
        faceRenderTargetView.Reset();
    }
    m_renderTargetView.Reset();
    m_shaderResourceView.Reset();
    m_texture.Reset();
    m_size = 0;
}

void EffectCubeMapResource::CreateRenderTargetViews(ID3D11Device* device, DXGI_FORMAT format,
                                                    bool createRenderTargetView, bool createFaceRenderTargetViews)
{
    if (createRenderTargetView)
    {
        D3D11_RENDER_TARGET_VIEW_DESC description{};
        description.Format = format;
        description.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        description.Texture2DArray.MipSlice = 0;
        description.Texture2DArray.FirstArraySlice = 0;
        description.Texture2DArray.ArraySize = 6;
        if (FAILED(device->CreateRenderTargetView(m_texture.Get(), &description, m_renderTargetView.GetAddressOf())))
        {
            Reset();
            throw std::runtime_error("Failed to create all-faces cubemap render-target view");
        }
    }

    if (!createFaceRenderTargetViews)
    {
        return;
    }
    for (std::uint32_t face = 0; face < m_faceRenderTargetViews.size(); ++face)
    {
        D3D11_RENDER_TARGET_VIEW_DESC description{};
        description.Format = format;
        description.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        description.Texture2DArray.MipSlice = 0;
        description.Texture2DArray.FirstArraySlice = face;
        description.Texture2DArray.ArraySize = 1;
        if (FAILED(device->CreateRenderTargetView(m_texture.Get(), &description,
                                                  m_faceRenderTargetViews[face].GetAddressOf())))
        {
            Reset();
            throw std::runtime_error("Failed to create cubemap face render-target view");
        }
    }
}

ID3D11Texture2D* EffectCubeMapResource::GetTexture() const noexcept
{
    return m_texture.Get();
}

ID3D11ShaderResourceView* EffectCubeMapResource::GetShaderResourceView() const noexcept
{
    return m_shaderResourceView.Get();
}

ID3D11RenderTargetView* EffectCubeMapResource::GetRenderTargetView() const noexcept
{
    return m_renderTargetView.Get();
}

ID3D11RenderTargetView* EffectCubeMapResource::GetFaceRenderTargetView(std::uint32_t face) const noexcept
{
    return face < m_faceRenderTargetViews.size() ? m_faceRenderTargetViews[face].Get() : nullptr;
}

std::uint32_t EffectCubeMapResource::GetSize() const noexcept
{
    return m_size;
}

bool EffectCubeMapResource::IsValid() const noexcept
{
    return m_texture != nullptr && m_shaderResourceView != nullptr;
}

} // namespace lrender
