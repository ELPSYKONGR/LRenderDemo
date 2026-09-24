/**
 * @file Effect resource factory and DX11 pipeline-state manager implementation.
 */
#include "stdfx.h"
#include "render/EffectManager.h"

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

std::unique_ptr<EffectManager> EffectManager::m_instance;

void EffectManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (m_instance != nullptr)
    {
        throw std::logic_error("EffectManager is already initialized");
    }
    m_instance = std::unique_ptr<EffectManager>(new EffectManager(device, context));
}

void EffectManager::Shutdown() noexcept
{
    m_instance.reset();
}

EffectManager& EffectManager::Instance()
{
    if (m_instance == nullptr)
    {
        throw std::logic_error("EffectManager is not initialized");
    }
    return *m_instance;
}

EffectManager::EffectManager(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_device(device), m_context(context)
{
    if (m_device == nullptr || m_context == nullptr)
    {
        throw std::invalid_argument("EffectManager requires a D3D11 device and context");
    }
    m_commonStates = std::make_unique<DirectX::CommonStates>(m_device);
    CreateCommonStates();
}

ID3D11Device* EffectManager::Device() const noexcept
{
    return m_device;
}

ID3D11DeviceContext* EffectManager::Context() const noexcept
{
    return m_context;
}

std::unique_ptr<EffectResource> EffectManager::CreateEffectResource(
    std::uint32_t width, std::uint32_t height, EffectResourceSizeMode sizeMode) const
{
    auto resource = std::make_unique<EffectResource>();
    CreateEffectResource(*resource, width, height, sizeMode);
    return resource;
}

void EffectManager::CreateEffectResource(EffectResource& resource,
                                         std::uint32_t width, std::uint32_t height,
                                         EffectResourceSizeMode sizeMode) const
{
    resource.SetSizeMode(sizeMode);
    resource.Resize(m_device, width, height);
}

EffectCubeMapResource EffectManager::CreateCubeMapResource(
    const std::filesystem::path& path, bool createRenderTargetView,
    bool createFaceRenderTargetViews, bool forceSrgb) const
{
    EffectCubeMapResource resource;
    resource.LoadDDS(m_device, path, createRenderTargetView,
                     createFaceRenderTargetViews, forceSrgb);
    return resource;
}

ID3D11DepthStencilState* EffectManager::GetDepthStencilState(DepthMode mode) const noexcept
{
    switch (mode)
    {
    case DepthMode::Disabled:
        return m_depthDisabledState.Get();
    case DepthMode::ReadOnly:
        return m_depthReadOnlyState.Get();
    case DepthMode::ReadWrite:
        return m_depthReadWriteState.Get();
    default:
        return nullptr;
    }
}

ID3D11BlendState* EffectManager::GetBlendState(BlendMode mode) const noexcept
{
    switch (mode)
    {
    case BlendMode::Opaque:
        return m_opaqueBlendState.Get();
    case BlendMode::AlphaBlend:
        return m_alphaBlendState.Get();
    case BlendMode::Additive:
        return m_additiveBlendState.Get();
    case BlendMode::Premultiplied:
        return m_premultipliedBlendState.Get();
    default:
        return nullptr;
    }
}

ID3D11RasterizerState* EffectManager::GetRasterizerState(RasterizerMode mode) const noexcept
{
    switch (mode)
    {
    case RasterizerMode::SolidCullClockwise:
        return m_solidCullClockwiseState.Get();
    case RasterizerMode::SolidCullNone:
        return m_solidCullNoneState.Get();
    case RasterizerMode::WireframeCullClockwise:
        return m_wireframeCullClockwiseState.Get();
    case RasterizerMode::WireframeCullNone:
        return m_wireframeCullNoneState.Get();
    case RasterizerMode::SolidCullClockwiseScissor:
        return m_solidCullClockwiseScissorState.Get();
    default:
        return nullptr;
    }
}

ID3D11SamplerState* EffectManager::GetLinearClampSampler() const noexcept
{
    return m_commonStates->LinearClamp();
}

void EffectManager::SetDepthMode(DepthMode mode)
{
    m_context->OMSetDepthStencilState(GetDepthStencilState(mode), 0);
}

void EffectManager::SetStencil(const StencilDescription& stencil)
{
    D3D11_DEPTH_STENCIL_DESC description = {};
    description.DepthEnable = TRUE;
    description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    description.StencilEnable = stencil.enabled ? TRUE : FALSE;
    description.StencilReadMask = stencil.readMask;
    description.StencilWriteMask = stencil.writeMask;
    description.FrontFace = {stencil.failOperation, stencil.depthFailOperation, stencil.passOperation, stencil.comparison};
    description.BackFace = description.FrontFace;
    const auto state = CreateDepthStencilState(description);
    m_context->OMSetDepthStencilState(state.Get(), stencil.reference);
}

void EffectManager::SetBlendMode(BlendMode mode, const std::array<float, 4>& blendFactor, UINT sampleMask)
{
    m_context->OMSetBlendState(GetBlendState(mode), blendFactor.data(), sampleMask);
}

void EffectManager::SetBlendState(const D3D11_BLEND_DESC& description,
                                  const std::array<float, 4>& blendFactor, UINT sampleMask)
{
    const auto state = CreateBlendState(description);
    m_context->OMSetBlendState(state.Get(), blendFactor.data(), sampleMask);
}

void EffectManager::SetRasterizerMode(RasterizerMode mode)
{
    m_context->RSSetState(GetRasterizerState(mode));
}

void EffectManager::SetRasterizerState(const D3D11_RASTERIZER_DESC& description)
{
    const auto state = CreateRasterizerState(description);
    m_context->RSSetState(state.Get());
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilState> EffectManager::CreateDepthStencilState(
    const D3D11_DEPTH_STENCIL_DESC& description) const
{
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;
    ThrowIfFailed(m_device->CreateDepthStencilState(&description, state.GetAddressOf()),
                  "Failed to create depth stencil state");
    return state;
}

Microsoft::WRL::ComPtr<ID3D11BlendState> EffectManager::CreateBlendState(
    const D3D11_BLEND_DESC& description) const
{
    Microsoft::WRL::ComPtr<ID3D11BlendState> state;
    ThrowIfFailed(m_device->CreateBlendState(&description, state.GetAddressOf()),
                  "Failed to create blend state");
    return state;
}

Microsoft::WRL::ComPtr<ID3D11RasterizerState> EffectManager::CreateRasterizerState(
    const D3D11_RASTERIZER_DESC& description) const
{
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> state;
    ThrowIfFailed(m_device->CreateRasterizerState(&description, state.GetAddressOf()),
                  "Failed to create rasterizer state");
    return state;
}

void EffectManager::CreateCommonStates()
{
    m_depthDisabledState = CreateDepthStencilState(BuildDepthStencilDescription(DepthMode::Disabled));
    m_depthReadOnlyState = CreateDepthStencilState(BuildDepthStencilDescription(DepthMode::ReadOnly));
    m_depthReadWriteState = CreateDepthStencilState(BuildDepthStencilDescription(DepthMode::ReadWrite));
    m_opaqueBlendState = CreateBlendState(BuildBlendDescription(BlendMode::Opaque));
    m_alphaBlendState = CreateBlendState(BuildBlendDescription(BlendMode::AlphaBlend));
    m_additiveBlendState = CreateBlendState(BuildBlendDescription(BlendMode::Additive));
    m_premultipliedBlendState = CreateBlendState(BuildBlendDescription(BlendMode::Premultiplied));
    m_solidCullClockwiseState = CreateRasterizerState(BuildRasterizerDescription(RasterizerMode::SolidCullClockwise));
    m_solidCullNoneState = CreateRasterizerState(BuildRasterizerDescription(RasterizerMode::SolidCullNone));
    m_wireframeCullClockwiseState = CreateRasterizerState(BuildRasterizerDescription(RasterizerMode::WireframeCullClockwise));
    m_wireframeCullNoneState = CreateRasterizerState(BuildRasterizerDescription(RasterizerMode::WireframeCullNone));
    m_solidCullClockwiseScissorState = CreateRasterizerState(BuildRasterizerDescription(RasterizerMode::SolidCullClockwiseScissor));
}

D3D11_DEPTH_STENCIL_DESC EffectManager::BuildDepthStencilDescription(DepthMode mode)
{
    D3D11_DEPTH_STENCIL_DESC description = {};
    description.DepthEnable = mode != DepthMode::Disabled ? TRUE : FALSE;
    description.DepthWriteMask = mode == DepthMode::ReadWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    return description;
}

D3D11_BLEND_DESC EffectManager::BuildBlendDescription(BlendMode mode)
{
    D3D11_BLEND_DESC description = {};
    D3D11_RENDER_TARGET_BLEND_DESC& target = description.RenderTarget[0];
    target.BlendOp = D3D11_BLEND_OP_ADD;
    target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    switch (mode)
    {
    case BlendMode::Opaque:
        target.SrcBlend = D3D11_BLEND_ONE;
        target.DestBlend = D3D11_BLEND_ZERO;
        target.SrcBlendAlpha = D3D11_BLEND_ONE;
        target.DestBlendAlpha = D3D11_BLEND_ZERO;
        break;
    case BlendMode::AlphaBlend:
        target.BlendEnable = TRUE;
        target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        target.SrcBlendAlpha = D3D11_BLEND_ONE;
        target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        break;
    case BlendMode::Additive:
        target.BlendEnable = TRUE;
        target.SrcBlend = D3D11_BLEND_ONE;
        target.DestBlend = D3D11_BLEND_ONE;
        target.SrcBlendAlpha = D3D11_BLEND_ONE;
        target.DestBlendAlpha = D3D11_BLEND_ONE;
        break;
    case BlendMode::Premultiplied:
        target.BlendEnable = TRUE;
        target.SrcBlend = D3D11_BLEND_ONE;
        target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        target.SrcBlendAlpha = D3D11_BLEND_ONE;
        target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        break;
    default:
        throw std::invalid_argument("Unsupported blend mode");
    }
    return description;
}

D3D11_RASTERIZER_DESC EffectManager::BuildRasterizerDescription(RasterizerMode mode)
{
    D3D11_RASTERIZER_DESC description = {};
    description.FillMode = D3D11_FILL_SOLID;
    description.CullMode = D3D11_CULL_BACK;
    description.FrontCounterClockwise = TRUE;
    description.DepthClipEnable = TRUE;

    switch (mode)
    {
    case RasterizerMode::SolidCullClockwise:
        break;
    case RasterizerMode::SolidCullNone:
        description.CullMode = D3D11_CULL_NONE;
        break;
    case RasterizerMode::WireframeCullClockwise:
        description.FillMode = D3D11_FILL_WIREFRAME;
        break;
    case RasterizerMode::WireframeCullNone:
        description.FillMode = D3D11_FILL_WIREFRAME;
        description.CullMode = D3D11_CULL_NONE;
        break;
    case RasterizerMode::SolidCullClockwiseScissor:
        description.ScissorEnable = TRUE;
        break;
    default:
        throw std::invalid_argument("Unsupported rasterizer mode");
    }
    return description;
}

} // namespace lrender
