/**
 * @file Editable HLSL basic mesh effect implementation.
 */
#include "render/BasicMeshEffect.h"
#include "render/CommonConstantBuffers.h"
#include "render/Mesh.h"

#include <iterator>
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

DirectX::SimpleMath::Vector4 ToVector4(const DirectX::SimpleMath::Color& color)
{
    return {color.x, color.y, color.z, color.w};
}

} // namespace

BasicMeshEffect::BasicMeshEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                                 const std::filesystem::path& shaderDirectory)
    : IRenderEffect(device, context), m_states(std::make_unique<DirectX::CommonStates>(Device()))
{
    const auto vertexShader = LoadShader(shaderDirectory / L"BasicMeshVS.cso");
    const auto pixelShader = LoadShader(shaderDirectory / L"BasicMeshPS.cso");

    ThrowIfFailed(Device()->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
                                               m_vertexShader.GetAddressOf()),
                  "Failed to create basic mesh vertex shader");
    ThrowIfFailed(Device()->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
                                              m_pixelShader.GetAddressOf()),
                  "Failed to create basic mesh pixel shader");
    constexpr D3D11_INPUT_ELEMENT_DESC inputElements[]{
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}};
    ThrowIfFailed(Device()->CreateInputLayout(inputElements, static_cast<UINT>(std::size(inputElements)),
                  vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(),
                                              m_inputLayout.GetAddressOf()),
                  "Failed to create basic mesh input layout");
}

std::string_view BasicMeshEffect::Name() const noexcept
{
    return "Basic Lit";
}

void BasicMeshEffect::SetWireframe(bool isWireframe) noexcept
{
    m_isWireframe = isWireframe;
}

bool BasicMeshEffect::IsWireframe() const noexcept
{
    return m_isWireframe;
}

LightingSettings& BasicMeshEffect::Lights() noexcept
{
    return m_lights;
}

const LightingSettings& BasicMeshEffect::Lights() const noexcept
{
    return m_lights;
}

void BasicMeshEffect::PrepareFrame(const EffectFrameContext& frame)
{
    LightConstants lightData{};
    lightData.ambientColor = ToVector4(m_lights.ambient);
    auto directionalDirection = m_lights.directional.direction;
    if (directionalDirection.LengthSquared() < 0.000001F)
    {
        directionalDirection = {0.0F, -1.0F, 0.0F};
    }
    else
    {
        directionalDirection.Normalize();
    }
    lightData.directionalDirectionAndIntensity = {directionalDirection.x, directionalDirection.y,
                                                  directionalDirection.z, m_lights.directional.intensity};
    lightData.directionalColorAndEnabled = {m_lights.directional.color.x, m_lights.directional.color.y,
                                            m_lights.directional.color.z, m_lights.directional.enabled ? 1.0F : 0.0F};
    for (std::size_t index = 0; index < m_lights.points.size(); ++index)
    {
        const PointLight& light = m_lights.points[index];
        lightData.pointLightData[index * 2] = {light.position.x, light.position.y, light.position.z,
                                               light.range > 0.0001F ? light.range : 0.0001F};
        lightData.pointLightData[index * 2 + 1] = {light.color.x, light.color.y, light.color.z,
                                                   light.enabled ? light.intensity : 0.0F};
    }
    lightData.pointLightCount = static_cast<std::uint32_t>(m_lights.points.size());
    frame.ConstantBuffers().UpdateLight(lightData);
    frame.ConstantBuffers().BindLight();
}

void BasicMeshEffect::Bind(const EffectFrameContext& frame, const EffectDrawContext& draw)
{
    ID3D11DeviceContext* context = frame.DeviceContext();
    const Material& material = draw.ResolvedMaterial();
    if (material.GetBaseColorTexture() == nullptr || material.GetSampler() == nullptr)
    {
        throw std::invalid_argument("BasicMeshEffect requires a texture and sampler material");
    }

    constexpr DirectX::SimpleMath::Color selectionColor{1.0F, 0.84F, 0.0F, 1.0F};
    const DirectX::SimpleMath::Color selectedTint =
        draw.IsSelected() ? DirectX::SimpleMath::Color::Lerp(draw.Tint(), selectionColor, 0.28F) : draw.Tint();
    const DirectX::SimpleMath::Color finalColor =
        material.UsesBaseColorTexture() ? DirectX::SimpleMath::Color(1.0F, 1.0F, 1.0F, 1.0F) : selectedTint;

    ObjectConstants objectData{};
    objectData.world = draw.World();
    objectData.worldViewProjection = objectData.world * frame.View() * frame.Projection();
    objectData.worldInverseTranspose = objectData.world.Invert().Transpose();

    MaterialConstants materialData{};
    materialData.baseColor = ToVector4(finalColor);
    materialData.specularColor = ToVector4(material.GetSpecularColor());
    materialData.materialParameters = {material.GetSpecularStrength(), material.GetShininess(),
                                       material.GetDiffuseStrength(), static_cast<float>(material.GetDisplayMode())};

    CommonConstantBuffers& buffers = frame.ConstantBuffers();
    buffers.UpdateObject(objectData);
    buffers.UpdateMaterial(materialData);

    context->IASetInputLayout(m_inputLayout.Get());
    // ColorProcessorEffect disables depth for its fullscreen pass. Restore the
    // scene depth test before every mesh draw so state does not leak between frames.
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_isWireframe ? m_states->Wireframe()
                                      : (material.IsDoubleSided() ? m_states->CullNone() : m_states->CullClockwise()));
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    buffers.BindFrame();
    buffers.BindObject();
    buffers.BindMaterial();
    buffers.BindLight();
    ID3D11ShaderResourceView* texture = material.GetBaseColorTexture()->ShaderResourceView();
    ID3D11SamplerState* sampler = material.GetSampler()->Get();
    context->PSSetShaderResources(0, 1, &texture);
    context->PSSetSamplers(0, 1, &sampler);
}

void BasicMeshEffect::Draw(const EffectFrameContext& frame, const EffectDrawContext& draw)
{
    Bind(frame, draw);
    const Mesh* mesh = draw.MeshGeometry();
    if (mesh == nullptr)
    {
        throw std::invalid_argument("BasicMeshEffect draw requires a mesh");
    }
    mesh->Draw(frame.DeviceContext());
}

} // namespace lrender
