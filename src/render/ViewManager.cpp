/**
 * @file View management and DX11 pipeline state protection implementation.
 */
#include "render/ViewManager.h"

#include <algorithm>
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
}

ViewId ViewManager::CreateView(std::uint32_t width, std::uint32_t height, HWND windowHandle)
{
    ViewInfo view;
    view.id = m_nextViewId++;
    view.width = std::max(width, 1U);
    view.height = std::max(height, 1U);
    view.windowHandle = windowHandle;
    view.target = std::make_unique<RenderTarget>();
    view.target->Resize(m_device, view.width, view.height);
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
    view->target->Resize(m_device, view->width, view->height);
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

void ViewManager::SetDepthMode(DepthMode mode)
{
    D3D11_DEPTH_STENCIL_DESC description{};
    description.DepthEnable = mode != DepthMode::Disabled;
    description.DepthWriteMask =
        mode == DepthMode::ReadWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    description.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;
    if (FAILED(m_device->CreateDepthStencilState(&description, state.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create depth state");
    }
    m_context->OMSetDepthStencilState(state.Get(), 0);
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
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> state;
    if (FAILED(m_device->CreateDepthStencilState(&description, state.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create stencil state");
    }
    m_context->OMSetDepthStencilState(state.Get(), stencil.reference);
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
