/**
 * @file View management and DX11 pipeline state snapshot protection implementation.
 */
#include "render/ViewManager.h"

#include "core/Scene.h"
#include "render/EffectManager.h"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace lrender
{

namespace
{

void ConfigureUntexturedMaterial(Entity& entity, const DirectX::SimpleMath::Color& color,
                                 float specularStrength = 0.12F, float shininess = 24.0F)
{
    EntityMaterial& material = entity.EntityMaterialData();
    material.baseColor = color;
    material.diffuseStrength = 1.0F;
    material.specularColor = {1.0F, 1.0F, 1.0F, 1.0F};
    material.specularStrength = specularStrength;
    material.shininess = shininess;
    material.displayMode = SurfaceDisplayMode::LitUntextured;
    material.useSourceTexture = false;
    material.baseColorTexturePath.clear();
    material.doubleSided = true;
}

} // namespace

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

ViewManager::ViewManager(ID3D11Device* device, ID3D11DeviceContext* context) : m_context(context)
{
    if (device == nullptr || m_context == nullptr)
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
    view.resource = EffectManager::Instance().CreateEffectResource(view.width, view.height);
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
    view->resource->Resize(EffectManager::Instance().Device(), view->width, view->height);
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

bool ViewManager::FitView(Camera& camera, const BoundingBox& boundingBox, float aspectRatio,
                          float margin) const noexcept
{
    return camera.CalcFitView(boundingBox, aspectRatio, margin);
}

void ViewManager::CreateSphereTestEntity(Scene& scene, std::uint32_t modelId) const
{
    constexpr std::uint32_t gridSize = 3;
    constexpr float radius = 0.5F;
    constexpr float spacing = 1.F;
    constexpr float height = radius;
    constexpr int center = static_cast<int>(radius / 2);
    std::mt19937 generator(20260909U);
    std::uniform_real_distribution<float> colorDistribution(0.15F, 0.95F);
    for (std::uint32_t z = 0; z < gridSize; ++z)
    {
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
                    static_cast<float>(static_cast<int>(z) - center) * spacing};

                EntityMaterial& material = entity.EntityMaterialData();
                material.baseColor = {colorDistribution(generator), colorDistribution(generator),colorDistribution(generator), 1.0F};
                material.displayMode = SurfaceDisplayMode::LitUntextured;
                material.useSourceTexture = false;
                material.baseColorTexturePath.clear();
                material.doubleSided = false;
            }
        }
    }
}

void ViewManager::CreatePlaneTestEntity(Scene& scene, std::uint32_t modelId) const
{
    constexpr float planeSize = 24.0F;
    constexpr float sphereGridBottom = -0.5F;
    constexpr float rearOffset = -3.0F;

    PlaneParameters parameters;
    parameters.size = {planeSize, planeSize};
    Entity& entity = scene.CreateSolidEntity(modelId, SolidGeometry::Plane(parameters), "SSRTestPlane");
    entity.transform.position = {0.0F, sphereGridBottom, rearOffset};

    EntityMaterial& material = entity.EntityMaterialData();
    material.baseColor = {1.F, 1.F, 1.F, 1.0F};
    material.specularStrength = 1.0F;
    material.shininess = 128.0F;
    material.displayMode = SurfaceDisplayMode::LitUntextured;
    material.useSourceTexture = false;
    material.baseColorTexturePath.clear();
    material.doubleSided = false;
}

void ViewManager::CreateCornellBoxTestScene(Scene& scene, std::uint32_t modelId) const
{
    constexpr DirectX::SimpleMath::Color floorColor{0.54F, 0.50F, 0.45F, 1.0F};
    constexpr DirectX::SimpleMath::Color ceilingColor{0.42F, 0.40F, 0.37F, 1.0F};
    constexpr DirectX::SimpleMath::Color backColor{0.36F, 0.35F, 0.33F, 1.0F};
    constexpr DirectX::SimpleMath::Color leftColor{0.58F, 0.035F, 0.025F, 1.0F};
    constexpr DirectX::SimpleMath::Color rightColor{0.025F, 0.42F, 0.07F, 1.0F};
    constexpr DirectX::SimpleMath::Color sphereColor{0.82F, 0.80F, 0.75F, 1.0F};
    constexpr DirectX::SimpleMath::Color lightColor{1.0F, 0.98F, 0.90F, 1.0F};
    constexpr float roomWidth = 6.0F;
    constexpr float roomHeight = 4.0F;
    constexpr float roomDepth = 6.0F;

    PlaneParameters floorParameters;
    floorParameters.size = {roomWidth, roomDepth};
    Entity& floor = scene.CreateSolidEntity(modelId, SolidGeometry::Plane(floorParameters), "CornellFloor");
    ConfigureUntexturedMaterial(floor, floorColor, 0.08F, 16.0F);

    Entity& ceiling = scene.CreateSolidEntity(modelId, SolidGeometry::Plane(floorParameters), "CornellCeiling");
    ceiling.transform.position = {0.0F, roomHeight, 0.0F};
    ceiling.transform.rotationDegrees.x = 180.0F;
    ConfigureUntexturedMaterial(ceiling, ceilingColor, 0.08F, 16.0F);

    PlaneParameters wallParameters;
    wallParameters.size = {roomWidth, roomHeight};
    Entity& backWall = scene.CreateSolidEntity(modelId, SolidGeometry::Plane(wallParameters), "CornellBackWall");
    backWall.transform.position = {0.0F, roomHeight * 0.5F, -roomDepth * 0.5F};
    backWall.transform.rotationDegrees.x = 90.0F;
    ConfigureUntexturedMaterial(backWall, backColor, 0.08F, 16.0F);
    wallParameters.size = {roomHeight, roomWidth};
    Entity& leftWall = scene.CreateSolidEntity(modelId, SolidGeometry::Plane(wallParameters), "CornellLeftWall");
    leftWall.transform.position = {-roomWidth * 0.5F, roomHeight * 0.5F, 0.0F};
    leftWall.transform.rotationDegrees.z = -90.0F;
    ConfigureUntexturedMaterial(leftWall, leftColor, 0.08F, 16.0F);

    Entity& rightWall = scene.CreateSolidEntity(modelId, SolidGeometry::Plane(wallParameters), "CornellRightWall");
    rightWall.transform.position = {roomWidth * 0.5F, roomHeight * 0.5F, 0.0F};
    rightWall.transform.rotationDegrees.z = 90.0F;
    ConfigureUntexturedMaterial(rightWall, rightColor, 0.08F, 16.0F);

    Entity& sphere = scene.CreateSolidEntity(
        modelId,
        SolidGeometry::Sphere({1.0F, 48, 32}),
        "CornellSphere");
    sphere.transform.position = {0.0F, 1.0F, -0.25F};
    ConfigureUntexturedMaterial(sphere, sphereColor, 0.35F, 64.0F);
    sphere.EntityMaterialData().doubleSided = false;

    PlaneParameters lightPanelParameters;
    lightPanelParameters.size = {1.8F, 0.9F};
    Entity& lightPanel = scene.CreateSolidEntity(
        modelId,
        SolidGeometry::Plane(lightPanelParameters),
        "CornellLightPanel");
    lightPanel.transform.position = {0.0F, roomHeight - 0.01F, 0.0F};
    lightPanel.transform.rotationDegrees.x = 180.0F;
    ConfigureUntexturedMaterial(lightPanel, lightColor, 0.0F, 8.0F);
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
