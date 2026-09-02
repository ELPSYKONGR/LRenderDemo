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
class Mesh;
struct Entity;

class EffectFrameContext final {
public:
    EffectFrameContext(
        ID3D11DeviceContext* deviceContext, const Camera& camera, float aspectRatio);

    [[nodiscard]] ID3D11DeviceContext* DeviceContext() const noexcept {
        return m_deviceContext;
    }
    [[nodiscard]] const DirectX::SimpleMath::Matrix& View() const noexcept { return m_view; }
    [[nodiscard]] const DirectX::SimpleMath::Matrix& Projection() const noexcept {
        return m_projection;
    }
    [[nodiscard]] const DirectX::SimpleMath::Vector3& CameraPosition() const noexcept {
        return m_cameraPosition;
    }
    [[nodiscard]] float AspectRatio() const noexcept { return m_aspectRatio; }

private:
    ID3D11DeviceContext* m_deviceContext = nullptr;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_projection;
    DirectX::SimpleMath::Vector3 m_cameraPosition;
    float m_aspectRatio = 0.0F;
};

class EffectDrawContext final {
public:
    EffectDrawContext(
        const Entity& entity, Material material, std::uint32_t selectedEntityId,
        const Mesh* mesh = nullptr);

    [[nodiscard]] const DirectX::SimpleMath::Matrix& World() const noexcept { return m_world; }
    [[nodiscard]] const Material& ResolvedMaterial() const noexcept { return m_material; }
    [[nodiscard]] const DirectX::SimpleMath::Color& Tint() const noexcept { return m_tint; }
    [[nodiscard]] bool IsSelected() const noexcept { return m_isSelected; }
    [[nodiscard]] const Mesh* MeshGeometry() const noexcept { return m_mesh; }

private:
    DirectX::SimpleMath::Matrix m_world;
    Material m_material;
    DirectX::SimpleMath::Color m_tint;
    bool m_isSelected = false;
    const Mesh* m_mesh = nullptr;
};

} // namespace lrender
