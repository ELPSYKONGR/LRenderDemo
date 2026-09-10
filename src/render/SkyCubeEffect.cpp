/**
 * @file Fullscreen TextureCube sky effect implementation.
 */

#include "stdfx.h"
#include "render/SkyCubeEffect.h"
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



SkyCubeEffect::SkyCubeEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                             const std::filesystem::path& shaderDirectory,
                             const std::filesystem::path& cubeMapPath)
    : IRenderEffect(device, context), m_states(std::make_unique<DirectX::CommonStates>(Device()))
{
    const auto vertexShader = LoadShader(shaderDirectory / L"SkyVS.cso");
    const auto pixelShader = LoadShader(shaderDirectory / L"SkyPS.cso");
    ThrowIfFailed(Device()->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
                                               m_vertexShader.GetAddressOf()),
                  "Failed to create sky vertex shader");
    ThrowIfFailed(Device()->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
                                              m_pixelShader.GetAddressOf()),
                  "Failed to create sky pixel shader");
    m_cubeMapResource.LoadDDS(Device(), cubeMapPath);
}

void SkyCubeEffect::Bind(const EffectFrameContext& frame)
{
    ID3D11DeviceContext* context = frame.DeviceContext();
    frame.ConstantBuffers().BindFrameBuffer();
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

void SkyCubeEffect::Bind(const EffectFrameContext& frame, const EffectDrawContext&)
{
    Bind(frame);
}

void SkyCubeEffect::Draw(const EffectFrameContext& frame)
{
    Bind(frame);
    //将天空盒载入 HLSL 的 TextureCube 中
    ID3D11DeviceContext* context = frame.DeviceContext();
    ID3D11ShaderResourceView* cubeMap = m_cubeMapResource.GetShaderResourceView();
    context->PSSetShaderResources(SkyTextureCubeSLOT, 1, &cubeMap);
    ID3D11SamplerState* sampler = m_states->LinearClamp();
    context->PSSetSamplers(LinearClampSamplerSLOT, 1, &sampler);
    //在光栅化阶段关闭背面消隐（正面是立方体向外的面，但摄像机在内部）
    context->RSSetState(m_states->CullNone());
    //在输出合并阶段设置深度/模板状态，深度比较函数为小于等于，以允许深度值为 1 的像素绘制
    D3D11_DEPTH_STENCIL_DESC description{};
    description.DepthEnable = TRUE;
    description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    const auto depthStencilState = ViewManager::Instance().CreateDepthStencilState(description);
    //绘制
    context->OMSetDepthStencilState(depthStencilState.Get(), 0);
    context->Draw(3, 0);
    //还原
    ID3D11ShaderResourceView* nullResource = nullptr;
    ID3D11SamplerState* nullSample = nullptr;
    context->PSSetShaderResources(SkyTextureCubeSLOT, 1, &nullResource);
    context->PSSetSamplers(LinearClampSamplerSLOT, 1, &nullSample);
}

void SkyCubeEffect::Draw(const EffectFrameContext& frame, const EffectDrawContext&)
{
    Draw(frame);
}

std::string_view SkyCubeEffect::Name() const noexcept
{
    return "Sky Cube";
}

const EffectCubeMapResource& SkyCubeEffect::CubeMapResource() const noexcept
{
    return m_cubeMapResource;
}

} // namespace lrender
