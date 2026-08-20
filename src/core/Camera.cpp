/**
 * @file Orbit editor camera implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Camera.h
 */
#include "core/Camera.h"

#include <algorithm>
#include <cmath>

namespace lrender {

void Camera::Orbit(float deltaX, float deltaY) {
    constexpr float sensitivity = 0.008F;
    yaw_ += deltaX * sensitivity;
    pitch_ = std::clamp(pitch_ + deltaY * sensitivity, -1.45F, 1.45F);
}

void Camera::Pan(float deltaX, float deltaY) {
    using DirectX::SimpleMath::Vector3;
    Vector3 forward = target_ - Position();
    forward.Normalize();
    Vector3 right = forward.Cross(Vector3::UnitY);
    right.Normalize();
    Vector3 up = right.Cross(forward);
    up.Normalize();
    const float scale = distance_ * 0.0015F;
    target_ += (-right * deltaX + up * deltaY) * scale;
}

void Camera::Zoom(float wheelDelta) {
    distance_ = std::clamp(distance_ * std::pow(0.85F, wheelDelta), 0.5F, 100.0F);
}

DirectX::SimpleMath::Vector3 Camera::Position() const {
    const float horizontal = distance_ * std::cos(pitch_);
    return target_ + DirectX::SimpleMath::Vector3{
                         horizontal * std::sin(yaw_),
                         distance_ * std::sin(pitch_),
                         horizontal * std::cos(yaw_)};
}

DirectX::SimpleMath::Matrix Camera::ViewMatrix() const {
    return DirectX::SimpleMath::Matrix::CreateLookAt(
        Position(), target_, DirectX::SimpleMath::Vector3::UnitY);
}

DirectX::SimpleMath::Matrix Camera::ProjectionMatrix(float aspectRatio) const {
    const float safeAspect = std::max(aspectRatio, 0.01F);
    return DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
        DirectX::XMConvertToRadians(60.0F), safeAspect, 0.05F, 500.0F);
}

} // namespace lrender
