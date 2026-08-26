/**
 * @file Validated frame and draw snapshots consumed by mesh effects.
 * @author Codex
 * @created 2026-08-26
 * @depends core/Camera.h, core/Scene.h, render/Material.h, D3D11
 */
#pragma once

#include "render/Material.h"

#include <SimpleMath.h>
#include <cstdint>
#include <d3d11.h>

namespace lrender {

class Camera;
struct Entity;

class EffectFrameContext final {
public:
    EffectFrameContext(
        ID3D11DeviceContext* deviceContext, const Camera& camera, float aspectRatio);

    [[nodiscard]] ID3D11DeviceContext* DeviceContext() const noexcept {
        return deviceContext_;
    }
    [[nodiscard]] const DirectX::SimpleMath::Matrix& View() const noexcept { return view_; }
    [[nodiscard]] const DirectX::SimpleMath::Matrix& Projection() const noexcept {
        return projection_;
    }
    [[nodiscard]] const DirectX::SimpleMath::Vector3& CameraPosition() const noexcept {
        return cameraPosition_;
    }

private:
    ID3D11DeviceContext* deviceContext_{};
    DirectX::SimpleMath::Matrix view_;
    DirectX::SimpleMath::Matrix projection_;
    DirectX::SimpleMath::Vector3 cameraPosition_;
};

class EffectDrawContext final {
public:
    EffectDrawContext(
        const Entity& entity, Material material, std::uint32_t selectedEntityId);

    [[nodiscard]] const DirectX::SimpleMath::Matrix& World() const noexcept { return world_; }
    [[nodiscard]] const Material& ResolvedMaterial() const noexcept { return material_; }
    [[nodiscard]] const DirectX::SimpleMath::Color& Tint() const noexcept { return tint_; }
    [[nodiscard]] bool IsSelected() const noexcept { return isSelected_; }

private:
    DirectX::SimpleMath::Matrix world_;
    Material material_;
    DirectX::SimpleMath::Color tint_;
    bool isSelected_{};
};

} // namespace lrender
