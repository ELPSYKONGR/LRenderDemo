/**
 * @file Effect frame and draw snapshot construction.
 * @author Codex
 * @created 2026-08-26
 * @depends render/EffectContext.h, core/Camera.h, core/Scene.h
 */
#include "render/EffectContext.h"

#include "core/Camera.h"
#include "core/Scene.h"
#include "render/CommonConstantBuffers.h"

#include <stdexcept>
#include <utility>

namespace lrender
{

EffectFrameContext::EffectFrameContext(ID3D11DeviceContext* deviceContext, CommonConstantBuffers& constantBuffers,
                                       const Camera& camera, float aspectRatio)
    : m_deviceContext(deviceContext), m_constantBuffers(&constantBuffers), m_view(camera.ViewMatrix()),
      m_projection(camera.ProjectionMatrix(aspectRatio)), m_inverseViewProjection((m_view * m_projection).Invert()),
      m_cameraPosition(camera.Position()), m_aspectRatio(aspectRatio)
{
    if (m_deviceContext == nullptr || m_constantBuffers == nullptr)
    {
        throw std::invalid_argument("EffectFrameContext requires a D3D11 context and constant buffers");
    }
}

EffectDrawContext::EffectDrawContext(const Entity& entity, Material material, std::uint32_t selectedEntityId,
                                     const Mesh* mesh)
    : m_world(entity.transform.ToMatrix()), m_material(std::move(material)),
      m_tint(entity.EffectiveMaterial().baseColor), m_isSelected(entity.id == selectedEntityId), m_mesh(mesh)
{
}

ID3D11DeviceContext* EffectFrameContext::DeviceContext() const noexcept
{
    return m_deviceContext;
}

CommonConstantBuffers& EffectFrameContext::ConstantBuffers() const noexcept
{
    return *m_constantBuffers;
}

const DirectX::SimpleMath::Matrix& EffectFrameContext::View() const noexcept
{
    return m_view;
}

const DirectX::SimpleMath::Matrix& EffectFrameContext::Projection() const noexcept
{
    return m_projection;
}

const DirectX::SimpleMath::Matrix& EffectFrameContext::InverseViewProjection() const noexcept
{
    return m_inverseViewProjection;
}

const DirectX::SimpleMath::Vector3& EffectFrameContext::CameraPosition() const noexcept
{
    return m_cameraPosition;
}

float EffectFrameContext::AspectRatio() const noexcept
{
    return m_aspectRatio;
}

RenderMode EffectFrameContext::GetRenderMode() const noexcept
{
    return m_renderMode;
}

void EffectFrameContext::SetRenderMode(RenderMode mode) noexcept
{
    m_renderMode = mode;
}

void EffectFrameContext::BeginFrame() const
{
    FrameConstants data{};
    data.view = m_view;
    data.projection = m_projection;
    data.inverseViewProjection = m_inverseViewProjection;
    data.cameraPosition = {m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, 1.0F};
    data.viewport = {m_aspectRatio, 1.0F, 0.0F, 0.0F};
    data.frameParameters = {static_cast<float>(m_renderMode), 0.0F, 0.0F, 0.0F};
    m_constantBuffers->UpdateFrameBuffer(data);
    m_constantBuffers->BindFrameBuffer();
}

const DirectX::SimpleMath::Matrix& EffectDrawContext::World() const noexcept
{
    return m_world;
}

const Material& EffectDrawContext::ResolvedMaterial() const noexcept
{
    return m_material;
}

const DirectX::SimpleMath::Color& EffectDrawContext::Tint() const noexcept
{
    return m_tint;
}

bool EffectDrawContext::IsSelected() const noexcept
{
    return m_isSelected;
}

const Mesh* EffectDrawContext::MeshGeometry() const noexcept
{
    return m_mesh;
}

} // namespace lrender
