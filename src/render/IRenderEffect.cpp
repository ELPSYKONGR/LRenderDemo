/**
 * @file Common DX11 render effect helpers.
 */
#include "render/IRenderEffect.h"

#include <d3dcompiler.h>
#include <filesystem>
#include <stdexcept>

namespace lrender
{

IRenderEffect::IRenderEffect(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_device(device), m_context(context)
{
    if (m_device == nullptr || m_context == nullptr)
    {
        throw std::invalid_argument("Render effect requires a D3D11 device and context");
    }
}

void IRenderEffect::Draw(const EffectFrameContext& frame, const EffectDrawContext& draw)
{
    Bind(frame, draw);
}

void IRenderEffect::ResizeResources(std::uint32_t, std::uint32_t)
{
}

ID3D11Device* IRenderEffect::Device() const noexcept
{
    return m_device;
}

ID3D11DeviceContext* IRenderEffect::DeviceContext() const noexcept
{
    return m_context;
}

Microsoft::WRL::ComPtr<ID3DBlob> IRenderEffect::LoadShader(const std::filesystem::path& path)
{
    Microsoft::WRL::ComPtr<ID3DBlob> shader;
    const HRESULT result = D3DReadFileToBlob(path.c_str(), shader.GetAddressOf());
    if (FAILED(result))
    {
        throw std::runtime_error("Failed to load compiled shader: " + path.string());
    }
    return shader;
}

} // namespace lrender
