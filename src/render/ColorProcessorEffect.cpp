/**
 * @file Fullscreen color processor effect implementation.
 */
#include "render/ColorProcessorEffect.h"

#include <stdexcept>

namespace lrender
{
namespace
{

void ThrowIfFailed(HRESULT result, const char* message)
{
    if (FAILED(result))
    {
        throw std::runtime_error(message);
    }
}

} // namespace

ColorProcessorEffect::ColorProcessorEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                                           const std::filesystem::path& shaderDirectory)
    : IRenderEffect(device, context), m_states(std::make_unique<DirectX::CommonStates>(Device()))
{
    const auto vertexShader = LoadShader(shaderDirectory / L"QuadViewVS.cso");
    const auto pixelShader = LoadShader(shaderDirectory / L"ColorProcessorPS.cso");
    ThrowIfFailed(Device()->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
                                               m_vertexShader.GetAddressOf()),
                  "Failed to create color processor vertex shader");
    ThrowIfFailed(Device()->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
                                              m_pixelShader.GetAddressOf()),
                  "Failed to create color processor pixel shader");
}

std::string_view ColorProcessorEffect::Name() const noexcept
{
    return "Color Processor";
}

void ColorProcessorEffect::Bind(const EffectFrameContext& frame, const EffectDrawContext&)
{
    SetPipeline(frame.DeviceContext());
}

void ColorProcessorEffect::Draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView* source)
{
    if (source == nullptr)
    {
        throw std::invalid_argument("Color processor requires a source texture");
    }
    SetPipeline(context);
    context->PSSetShaderResources(0, 1, &source);
    context->Draw(3, 0);
    ID3D11ShaderResourceView* nullResource = nullptr;
    context->PSSetShaderResources(0, 1, &nullResource);
}

void ColorProcessorEffect::SetPipeline(ID3D11DeviceContext* context)
{
    if (context == nullptr)
    {
        throw std::invalid_argument("Color processor requires a D3D11 context");
    }
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    ID3D11SamplerState* sampler = m_states->LinearClamp();
    context->PSSetSamplers(0, 1, &sampler);
}

} // namespace lrender
