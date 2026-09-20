/**
 * @file Fullscreen TextureCube sky effect implementation.
 */

#include "stdfx.h"
#include "render/EffectManager.h"
#include "render/SkyCubeEffect.h"
#include <stdexcept>

namespace lrender
{




SkyCubeEffect::SkyCubeEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                             const std::filesystem::path& shaderDirectory,
                             const std::filesystem::path& cubeMapPath)
    : IRenderEffect(device, context)
{
    const auto vertexShader = LoadShader(shaderDirectory / L"SkyVS.cso");
    const auto pixelShader = LoadShader(shaderDirectory / L"SkyPS.cso");
    ThrowIfFailed(Device()->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
                                               m_vertexShader.GetAddressOf()),
                  "Failed to create sky vertex shader");
    ThrowIfFailed(Device()->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
                                              m_pixelShader.GetAddressOf()),
                  "Failed to create sky pixel shader");
    m_cubeMapResource = EffectManager::Instance().CreateCubeMapResource(cubeMapPath);
}

void SkyCubeEffect::BindPipeline(const EffectFrameContext& frame)
{
    ID3D11DeviceContext* context = frame.DeviceContext();
    frame.ConstantBuffers().BindFrameBuffer();
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

void SkyCubeEffect::RenderEffect(const EffectFrameContext& frame)
{
    BindPipeline(frame);
    //将天空盒载入 HLSL 的 TextureCube 中
    ID3D11DeviceContext* context = frame.DeviceContext();
    ID3D11ShaderResourceView* cubeMap = m_cubeMapResource.GetShaderResourceView();
    context->PSSetShaderResources(SkyTextureCubeSLOT, 1, &cubeMap);
    EffectManager& effectManager = EffectManager::Instance();
    ID3D11SamplerState* sampler = effectManager.GetLinearClampSampler();
    context->PSSetSamplers(LinearClampSamplerSLOT, 1, &sampler);
    //在光栅化阶段关闭背面消隐（正面是立方体向外的面，但摄像机在内部）
    effectManager.SetRasterizerMode(RasterizerMode::SolidCullNone);
    //在输出合并阶段设置深度/模板状态，深度比较函数为小于等于，以允许深度值为 1 的像素绘制
	D3D11_DEPTH_STENCIL_DESC description{};
	description.DepthEnable = TRUE;
	description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    const auto depthStencilState = effectManager.CreateDepthStencilState(description);
    context->OMSetDepthStencilState(depthStencilState.Get(),0);
    //绘制
    context->Draw(3, 0);
    //还原
    ID3D11ShaderResourceView* nullResource = nullptr;
    ID3D11SamplerState* nullSample = nullptr;
    context->PSSetShaderResources(SkyTextureCubeSLOT, 1, &nullResource);
    context->PSSetSamplers(LinearClampSamplerSLOT, 1, &nullSample);
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
