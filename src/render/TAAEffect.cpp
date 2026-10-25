/**
 * @file Temporal anti-aliasing accumulation effect implementation.
 */
#include "stdfx.h"
#include "render/EffectManager.h"
#include "render/TAAEffect.h"

#include <stdexcept>

namespace lrender
{

TAAEffect::TAAEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                     const std::filesystem::path& shaderDirectory)
    : IRenderEffect(device, context), m_constantBuffer(device)
{
    const auto vertexShader = LoadShader(shaderDirectory / L"TAAEffectVS.cso");
    const auto pixelShader = LoadShader(shaderDirectory / L"TAAEffectPS.cso");
    if (FAILED(Device()->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
                                             m_vertexShader.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create TAA vertex shader");
    }
    if (FAILED(Device()->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
                                            m_pixelShader.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create TAA pixel shader");
    }
    ResizeResources(1, 1);
}

void TAAEffect::SetSource(ID3D11ShaderResourceView* source)
{
    if (source == nullptr)
    {
        throw std::invalid_argument("TAA effect requires a source texture");
    }
    m_source = source;
}

void TAAEffect::ResetHistory() noexcept
{
    m_historyValid = false;
}

void TAAEffect::BindPipeline(const EffectFrameContext& frame)
{
    ID3D11DeviceContext* context = frame.DeviceContext();
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    EffectManager& effectManager = EffectManager::Instance();
    effectManager.SetDepthMode(DepthMode::Disabled);
    effectManager.SetBlendMode(BlendMode::Opaque);
    effectManager.SetRasterizerMode(RasterizerMode::SolidCullNone);
    ID3D11SamplerState* sampler = effectManager.GetLinearClampSampler();
    context->PSSetSamplers(LinearClampSamplerSLOT, 1, &sampler);
}

void TAAEffect::RenderEffect(const EffectFrameContext& frame)
{
    if (!m_source || m_historyResources[0].GetShaderResourceView() == nullptr ||
        m_historyResources[1].GetShaderResourceView() == nullptr)
    {
        throw std::logic_error("TAA effect resources are not ready");
    }

    const std::uint32_t writeIndex = 1U - m_historyIndex;
    const EffectResource& readResource = m_historyResources[m_historyIndex];
    EffectResource& writeResource = m_historyResources[writeIndex];
    constexpr float clearColor[4] = {0.0F, 0.0F, 0.0F, 1.0F};
    writeResource.BindAndClear(frame.DeviceContext(), clearColor);
    BindPipeline(frame);

    TAAConstants constants{};
    constants.parameters = {0.10F, m_historyValid ? 1.0F : 0.0F,
                            1.0F / static_cast<float>(writeResource.GetWidth()),
                            1.0F / static_cast<float>(writeResource.GetHeight())};
    m_constantBuffer.Update(frame.DeviceContext(), constants);
    m_constantBuffer.BindPS(frame.DeviceContext(), TAAInfoSLOT);

    ID3D11ShaderResourceView* current = m_source.Get();
    ID3D11ShaderResourceView* history = m_historyValid ? readResource.GetShaderResourceView() : current;
    ID3D11DeviceContext* context = frame.DeviceContext();
    context->PSSetShaderResources(ColorSLOT, 1, &current);
    context->PSSetShaderResources(TAAHistorySLOT, 1, &history);
    context->Draw(3, 0);

    ID3D11ShaderResourceView* nullResource = nullptr;
    context->PSSetShaderResources(ColorSLOT, 1, &nullResource);
    context->PSSetShaderResources(TAAHistorySLOT, 1, &nullResource);
    ID3D11SamplerState* nullSampler = nullptr;
    context->PSSetSamplers(LinearClampSamplerSLOT, 1, &nullSampler);
    m_source.Reset();
    m_historyIndex = writeIndex;
    m_historyValid = true;
}

void TAAEffect::ResizeResources(std::uint32_t width, std::uint32_t height)
{
    const bool sizeChanged = m_historyResources[0].GetWidth() != width || m_historyResources[0].GetHeight() != height;
    EffectManager& effectManager = EffectManager::Instance();
    effectManager.CreateEffectResource(m_historyResources[0], width, height);
    effectManager.CreateEffectResource(m_historyResources[1], width, height);
    if (sizeChanged)
    {
        m_historyIndex = 0;
        m_historyValid = false;
    }
}

std::string_view TAAEffect::Name() const noexcept
{
    return "TAA";
}

const EffectResource& TAAEffect::OutputResource() const noexcept
{
    return m_historyResources[m_historyIndex];
}

} // namespace lrender
