/**
 * @file View management and DX11 pipeline state protection implementation.
 */
#include "render/ViewManager.h"

#include "core/Scene.h"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace lrender
{

std::unique_ptr<ViewManager> ViewManager::m_instance;

ViewStateGuard::ViewStateGuard(ID3D11DeviceContext* context) : m_context(context)
{
    if (m_context == nullptr)
    {
        throw std::invalid_argument("ViewStateGuard requires a D3D11 context");
    }
    ID3D11RenderTargetView* targets[8]{};
    ID3D11DepthStencilView* depthStencilView{};
    m_context->OMGetRenderTargets(8, targets, &depthStencilView);
    for (std::size_t index = 0; index < m_renderTargets.size(); ++index)
    {
        m_renderTargets[index].Attach(targets[index]);
    }
    m_depthStencilView.Attach(depthStencilView);

    ID3D11VertexShader* vertexShader{};
    ID3D11HullShader* hullShader{};
    ID3D11DomainShader* domainShader{};
    ID3D11GeometryShader* geometryShader{};
    ID3D11PixelShader* pixelShader{};
    ID3D11ComputeShader* computeShader{};
    ID3D11InputLayout* inputLayout{};
    ID3D11RasterizerState* rasterizerState{};
    ID3D11DepthStencilState* depthStencilState{};
    ID3D11BlendState* blendState{};
    m_context->VSGetShader(&vertexShader, nullptr, nullptr);
    m_context->HSGetShader(&hullShader, nullptr, nullptr);
    m_context->DSGetShader(&domainShader, nullptr, nullptr);
    m_context->GSGetShader(&geometryShader, nullptr, nullptr);
    m_context->PSGetShader(&pixelShader, nullptr, nullptr);
    m_context->CSGetShader(&computeShader, nullptr, nullptr);
    m_context->IAGetInputLayout(&inputLayout);
    m_context->RSGetState(&rasterizerState);
    m_context->OMGetDepthStencilState(&depthStencilState, &m_stencilReference);
    m_context->OMGetBlendState(&blendState, m_blendFactor.data(), &m_sampleMask);
    m_vertexShader.Attach(vertexShader);
    m_hullShader.Attach(hullShader);
    m_domainShader.Attach(domainShader);
    m_geometryShader.Attach(geometryShader);
    m_pixelShader.Attach(pixelShader);
    m_computeShader.Attach(computeShader);
    m_inputLayout.Attach(inputLayout);
    m_rasterizerState.Attach(rasterizerState);
    m_depthStencilState.Attach(depthStencilState);
    m_blendState.Attach(blendState);
    m_context->IAGetPrimitiveTopology(&m_primitiveTopology);
    m_viewportCount = static_cast<UINT>(m_viewports.size());
    m_context->RSGetViewports(&m_viewportCount, m_viewports.data());
}

ViewStateGuard::~ViewStateGuard()
{
    if (m_context == nullptr)
    {
        return;
    }
    ID3D11RenderTargetView* targets[8]{};
    for (std::size_t index = 0; index < m_renderTargets.size(); ++index)
    {
        targets[index] = m_renderTargets[index].Get();
    }
    m_context->OMSetRenderTargets(8, targets, m_depthStencilView.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->HSSetShader(m_hullShader.Get(), nullptr, 0);
    m_context->DSSetShader(m_domainShader.Get(), nullptr, 0);
    m_context->GSSetShader(m_geometryShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->CSSetShader(m_computeShader.Get(), nullptr, 0);
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->RSSetState(m_rasterizerState.Get());
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), m_stencilReference);
    m_context->OMSetBlendState(m_blendState.Get(), m_blendFactor.data(), m_sampleMask);
    m_context->IASetPrimitiveTopology(m_primitiveTopology);
    m_context->RSSetViewports(m_viewportCount, m_viewports.data());
}

void ViewManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (m_instance != nullptr)
    {
        throw std::logic_error("ViewManager is already initialized");
    }
    m_instance = std::unique_ptr<ViewManager>(new ViewManager(device, context));
}

void ViewManager::Shutdown() noexcept
{
    m_instance.reset();
}

ViewManager& ViewManager::Instance()
{
    if (m_instance == nullptr)
    {
        throw std::logic_error("ViewManager is not initialized");
    }
    return *m_instance;
}

ViewManager::ViewManager(ID3D11Device* device, ID3D11DeviceContext* context) : m_device(device), m_context(context)
{
    if (m_device == nullptr || m_context == nullptr)
    {
        throw std::invalid_argument("ViewManager requires a D3D11 device and context");
    }
    CreateCommonStates();
}

ViewId ViewManager::CreateView(std::uint32_t width, std::uint32_t height, HWND windowHandle)
{
    ViewInfo view;
    view.id = m_nextViewId++;
    view.width = std::max(width, 1U);
    view.height = std::max(height, 1U);
    view.windowHandle = windowHandle;
    view.resource = std::make_unique<EffectResource>();
    view.resource->Resize(m_device, view.width, view.height);
    const ViewId id = view.id;
    m_views.emplace(id, std::move(view));
    if (m_activeViewId == 0)
    {
        m_activeViewId = id;
    }
    return id;
}

bool ViewManager::RemoveView(ViewId id) noexcept
{
    if (m_views.erase(id) == 0)
    {
        return false;
    }
    if (m_activeViewId == id)
    {
        m_activeViewId = m_views.empty() ? 0 : m_views.begin()->first;
    }
    return true;
}

ViewInfo* ViewManager::FindView(ViewId id) noexcept
{
    const auto iterator = m_views.find(id);
    return iterator == m_views.end() ? nullptr : &iterator->second;
}

const ViewInfo* ViewManager::FindView(ViewId id) const noexcept
{
    const auto iterator = m_views.find(id);
    return iterator == m_views.end() ? nullptr : &iterator->second;
}

void ViewManager::ResizeView(ViewId id, std::uint32_t width, std::uint32_t height)
{
    ViewInfo* view = FindView(id);
    if (view == nullptr)
    {
        throw std::invalid_argument("View does not exist");
    }
    view->width = std::max(width, 1U);
    view->height = std::max(height, 1U);
    view->resource->Resize(m_device, view->width, view->height);
}

void ViewManager::AttachWindow(ViewId id, HWND windowHandle)
{
    ViewInfo* view = FindView(id);
    if (view == nullptr)
    {
        throw std::invalid_argument("View does not exist");
    }
    view->windowHandle = windowHandle;
}

void ViewManager::SetActiveView(ViewId id)
{
    if (FindView(id) == nullptr)
    {
        throw std::invalid_argument("View does not exist");
    }
    m_activeViewId = id;
}

std::unique_ptr<ViewStateGuard> ViewManager::CaptureState() const
{
    return std::make_unique<ViewStateGuard>(m_context);
}

void ViewManager::CreateSphereTestEntity(Scene& scene, std::uint32_t modelId) const
{
    constexpr std::uint32_t gridSize = 9;
    constexpr float radius = 0.5F;
    constexpr float spacing = 1.5F;
    constexpr float height = radius;
    constexpr int center = static_cast<int>(gridSize / 2);
    std::mt19937 generator(20260909U);
    std::uniform_real_distribution<float> colorDistribution(0.15F, 0.95F);

    for (std::uint32_t row = 0; row < gridSize; ++row)
    {
        for (std::uint32_t column = 0; column < gridSize; ++column)
        {
            const std::string name =
                "SphereTest_" + std::to_string(row + 1) + "_" + std::to_string(column + 1);
            Entity& entity = scene.CreateEntity(modelId, PrimitiveType::Sphere, name);
            entity.transform.position = {
                static_cast<float>(static_cast<int>(column) - center) * spacing
                ,static_cast<float>(static_cast<int>(row) - center) * spacing,
                height};

            EntityMaterial& material = entity.EntityMaterialData();
            material.baseColor = {colorDistribution(generator), colorDistribution(generator),
                                  colorDistribution(generator), 1.0F};
            material.displayMode = SurfaceDisplayMode::LitUntextured;
            material.useSourceTexture = false;
            material.baseColorTexturePath.clear();
            material.doubleSided = false;
        }
    }
}

ID3D11DepthStencilState* ViewManager::GetDepthStencilState(DepthMode mode) const noexcept
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

ID3D11BlendState* ViewManager::GetBlendState(BlendMode mode) const noexcept
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

void ViewManager::SetDepthMode(DepthMode mode)
{
    m_context->OMSetDepthStencilState(GetDepthStencilState(mode), 0);
}

void ViewManager::SetStencil(const StencilDescription& stencil)
{
    D3D11_DEPTH_STENCIL_DESC description{};
    description.DepthEnable = TRUE;
    description.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    description.StencilEnable = stencil.enabled ? TRUE : FALSE;
    description.StencilReadMask = stencil.readMask;
    description.StencilWriteMask = stencil.writeMask;
    description.FrontFace = {stencil.failOperation, stencil.depthFailOperation, stencil.passOperation,
                             stencil.comparison};
    description.BackFace = description.FrontFace;
    const auto state = CreateDepthStencilState(description);
    m_context->OMSetDepthStencilState(state.Get(), stencil.reference);
}

void ViewManager::SetBlendMode(BlendMode mode, const std::array<float, 4>& blendFactor, UINT sampleMask)
{
    const auto state = CreateBlendState(BuildBlendDescription(mode));
    m_context->OMSetBlendState(state.Get(), blendFactor.data(), sampleMask);
}

void ViewManager::SetBlendState(const D3D11_BLEND_DESC& description,
                                const std::array<float, 4>& blendFactor, UINT sampleMask)
{
    const auto state = CreateBlendState(description);
    m_context->OMSetBlendState(state.Get(), blendFactor.data(), sampleMask);
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilState> ViewManager::CreateDepthStencilState(
    const D3D11_DEPTH_STENCIL_DESC& description) const
{
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;
    if (FAILED(m_device->CreateDepthStencilState(&description, state.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create depth stencil state");
    }
    return state;
}

Microsoft::WRL::ComPtr<ID3D11BlendState> ViewManager::CreateBlendState(const D3D11_BLEND_DESC& description) const
{
    Microsoft::WRL::ComPtr<ID3D11BlendState> state;
    if (FAILED(m_device->CreateBlendState(&description, state.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create blend state");
    }
    return state;
}

void ViewManager::CreateCommonStates()
{
    m_depthDisabledState = CreateDepthStencilState(BuildDepthStencilDescription(DepthMode::Disabled));
    m_depthReadOnlyState = CreateDepthStencilState(BuildDepthStencilDescription(DepthMode::ReadOnly));
    m_depthReadWriteState = CreateDepthStencilState(BuildDepthStencilDescription(DepthMode::ReadWrite));
    m_opaqueBlendState = CreateBlendState(BuildBlendDescription(BlendMode::Opaque));
    m_alphaBlendState = CreateBlendState(BuildBlendDescription(BlendMode::AlphaBlend));
    m_additiveBlendState = CreateBlendState(BuildBlendDescription(BlendMode::Additive));
    m_premultipliedBlendState = CreateBlendState(BuildBlendDescription(BlendMode::Premultiplied));
}

D3D11_DEPTH_STENCIL_DESC ViewManager::BuildDepthStencilDescription(DepthMode mode)
{
    D3D11_DEPTH_STENCIL_DESC description{};
    description.DepthEnable = mode != DepthMode::Disabled ? TRUE : FALSE;
    description.DepthWriteMask = mode == DepthMode::ReadWrite ? D3D11_DEPTH_WRITE_MASK_ALL
                                                               : D3D11_DEPTH_WRITE_MASK_ZERO;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    return description;
}

D3D11_BLEND_DESC ViewManager::BuildBlendDescription(BlendMode mode)
{
    D3D11_BLEND_DESC description{};
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

ViewId ViewManager::ActiveViewId() const noexcept
{
    return m_activeViewId;
}

ViewInfo* ViewManager::ActiveView() noexcept
{
    return FindView(m_activeViewId);
}

std::size_t ViewManager::ViewCount() const noexcept
{
    return m_views.size();
}

} // namespace lrender
