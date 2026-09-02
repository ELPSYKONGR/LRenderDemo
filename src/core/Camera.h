/**
 * @file Orbit editor camera independent from Win32 input handling.
 * @author Codex
 * @created 2026-08-20
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>

namespace lrender
{

enum class CameraViewPreset
{
    Front,
    Back,
    Left,
    Right,
    Top,
    Bottom,
    RightIsometric,
    LeftIsometric
};

class Camera final
{
  public:
    void Orbit(float deltaX, float deltaY);
    void Pan(float deltaX, float deltaY);
    void Zoom(float wheelDelta);
    void SetView(CameraViewPreset preset) noexcept;
    void RotateAroundTarget(float deltaDegrees) noexcept;

    [[nodiscard]] DirectX::SimpleMath::Matrix ViewMatrix() const;
    [[nodiscard]] DirectX::SimpleMath::Matrix ProjectionMatrix(float aspectRatio) const;
    [[nodiscard]] DirectX::SimpleMath::Vector3 Position() const;

  private:
    [[nodiscard]] DirectX::SimpleMath::Vector3 UpDirection() const noexcept;

    DirectX::SimpleMath::Vector3 m_target = {0.0F, 0.0F, 0.0F};
    float m_yaw = DirectX::XMConvertToRadians(35.0F);
    float m_pitch = DirectX::XMConvertToRadians(-20.0F);
    float m_distance = 8.0F;
};

} // namespace lrender
