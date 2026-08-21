/**
 * @file Orbit editor camera independent from Win32 input handling.
 * @author Codex
 * @created 2026-08-20
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>

namespace lrender {

class Camera final {
public:
    void Orbit(float deltaX, float deltaY);
    void Pan(float deltaX, float deltaY);
    void Zoom(float wheelDelta);

    [[nodiscard]] DirectX::SimpleMath::Matrix ViewMatrix() const;
    [[nodiscard]] DirectX::SimpleMath::Matrix ProjectionMatrix(float aspectRatio) const;
    [[nodiscard]] DirectX::SimpleMath::Vector3 Position() const;

private:
    DirectX::SimpleMath::Vector3 target_{0.0F, 0.0F, 0.0F};
    float yaw_{DirectX::XMConvertToRadians(35.0F)};
    float pitch_{DirectX::XMConvertToRadians(-20.0F)};
    float distance_{8.0F};
};

} // namespace lrender
