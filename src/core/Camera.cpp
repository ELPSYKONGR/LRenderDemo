/**
 * @file Orbit editor camera implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Camera.h
 */
#include "core/Camera.h"

#include <algorithm>
#include <cmath>

namespace lrender
{

namespace
{

constexpr float isometricPitch = 0.6154797F; // asin(1 / sqrt(3))

} // namespace

void Camera::Orbit(float deltaX, float deltaY)
{
    constexpr float sensitivity = 0.008F;
    m_yaw += deltaX * sensitivity;
    m_pitch = std::clamp(m_pitch + deltaY * sensitivity, -1.45F, 1.45F);
}

void Camera::Pan(float deltaX, float deltaY)
{
    using DirectX::SimpleMath::Vector3;
    Vector3 forward = m_target - Position();
    forward.Normalize();
    Vector3 right = forward.Cross(UpDirection());
    right.Normalize();
    Vector3 up = right.Cross(forward);
    up.Normalize();
    const float scale = m_distance * 0.0015F;
    m_target += (-right * deltaX + up * deltaY) * scale;
}

void Camera::Zoom(float wheelDelta)
{
    m_distance = std::clamp(m_distance * std::pow(0.85F, wheelDelta), 0.5F, 100.0F);
}

void Camera::SetView(CameraViewPreset preset) noexcept
{
    switch (preset)
    {
    case CameraViewPreset::Front:
        m_yaw = 0.0F;
        m_pitch = 0.0F;
        break;
    case CameraViewPreset::Back:
        m_yaw = DirectX::XM_PI;
        m_pitch = 0.0F;
        break;
    case CameraViewPreset::Left:
        m_yaw = -DirectX::XM_PIDIV2;
        m_pitch = 0.0F;
        break;
    case CameraViewPreset::Right:
        m_yaw = DirectX::XM_PIDIV2;
        m_pitch = 0.0F;
        break;
    case CameraViewPreset::Top:
        m_yaw = 0.0F;
        m_pitch = DirectX::XM_PIDIV2;
        break;
    case CameraViewPreset::Bottom:
        m_yaw = 0.0F;
        m_pitch = -DirectX::XM_PIDIV2;
        break;
    case CameraViewPreset::RightIsometric:
        m_yaw = DirectX::XM_PIDIV4;
        m_pitch = isometricPitch;
        break;
    case CameraViewPreset::LeftIsometric:
        m_yaw = -DirectX::XM_PIDIV4;
        m_pitch = isometricPitch;
        break;
    }
}

void Camera::RotateAroundTarget(float deltaDegrees) noexcept
{
    m_yaw = std::remainder(m_yaw + DirectX::XMConvertToRadians(deltaDegrees), DirectX::XM_2PI);
}

DirectX::SimpleMath::Vector3 Camera::Position() const
{
    const float horizontal = m_distance * std::cos(m_pitch);
    return m_target + DirectX::SimpleMath::Vector3{horizontal * std::sin(m_yaw), m_distance * std::sin(m_pitch),
                                                   horizontal * std::cos(m_yaw)};
}

DirectX::SimpleMath::Matrix Camera::ViewMatrix() const
{
    return DirectX::SimpleMath::Matrix::CreateLookAt(Position(), m_target, UpDirection());
}

DirectX::SimpleMath::Vector3 Camera::UpDirection() const noexcept
{
    if (std::abs(std::cos(m_pitch)) < 0.001F)
    {
        return m_pitch > 0.0F ? -DirectX::SimpleMath::Vector3::UnitZ : DirectX::SimpleMath::Vector3::UnitZ;
    }
    return DirectX::SimpleMath::Vector3::UnitY;
}

DirectX::SimpleMath::Matrix Camera::ProjectionMatrix(float aspectRatio) const
{
    const float safeAspect = std::max(aspectRatio, 0.01F);
    return DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(DirectX::XMConvertToRadians(60.0F), safeAspect,
                                                                     0.05F, 500.0F);
}

} // namespace lrender
