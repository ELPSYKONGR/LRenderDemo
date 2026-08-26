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
    : deviceContext_(deviceContext),
      view_(camera.ViewMatrix()),
      projection_(camera.ProjectionMatrix(aspectRatio)),
      cameraPosition_(camera.Position()) {
    if (deviceContext_ == nullptr) {
        throw std::invalid_argument("EffectFrameContext requires a D3D11 context");
    }
}

EffectDrawContext::EffectDrawContext(
    const Entity& entity, Material material, std::uint32_t selectedEntityId)
    : world_(entity.transform.ToMatrix()),
      material_(std::move(material)),
      tint_(entity.material.baseColor),
      isSelected_(entity.id == selectedEntityId) {}

} // namespace lrender
