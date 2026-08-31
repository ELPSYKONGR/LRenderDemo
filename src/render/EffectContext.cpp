/**
 * @file Effect frame and draw snapshot construction.
 * @author Codex
 * @created 2026-08-26
 * @depends render/EffectContext.h, core/Camera.h, core/Scene.h
 */
#include "render/EffectContext.h"

#include "core/Camera.h"
#include "core/Scene.h"

#include <stdexcept>
#include <utility>

namespace lrender {

EffectFrameContext::EffectFrameContext(
    ID3D11DeviceContext* deviceContext, const Camera& camera, float aspectRatio)
    : m_deviceContext(deviceContext),
      m_view(camera.ViewMatrix()),
      m_projection(camera.ProjectionMatrix(aspectRatio)),
      m_cameraPosition(camera.Position()),
      m_aspectRatio(aspectRatio) {
    if (m_deviceContext == nullptr) {
        throw std::invalid_argument("EffectFrameContext requires a D3D11 context");
    }
}

EffectDrawContext::EffectDrawContext(
    const Entity& entity, Material material, std::uint32_t selectedEntityId)
    : m_world(entity.transform.ToMatrix()),
      m_material(std::move(material)),
      m_tint(entity.material.baseColor),
      m_isSelected(entity.id == selectedEntityId) {}

} // namespace lrender
