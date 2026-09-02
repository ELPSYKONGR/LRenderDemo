/**
 * @file Editable object transform shared by scene, UI, and commands.
 * @author Codex
 * @created 2026-08-20
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>

#include <cmath>

namespace lrender {

struct Transform {
    DirectX::SimpleMath::Vector3 position = {0.0F, 0.0F, 0.0F};
    DirectX::SimpleMath::Vector3 rotationDegrees = {0.0F, 0.0F, 0.0F};
    DirectX::SimpleMath::Vector3 scale = {1.0F, 1.0F, 1.0F};

    /** Builds a row-major world matrix suitable for DirectXMath and ImGuizmo. */
    [[nodiscard]] DirectX::SimpleMath::Matrix ToMatrix() const {
        using DirectX::SimpleMath::Matrix;
        return Matrix::CreateScale(scale) *
               Matrix::CreateFromYawPitchRoll(
                   DirectX::XMConvertToRadians(rotationDegrees.y),
                   DirectX::XMConvertToRadians(rotationDegrees.x),
                   DirectX::XMConvertToRadians(rotationDegrees.z)) *
               Matrix::CreateTranslation(position);
    }

    [[nodiscard]] bool NearlyEquals(const Transform& other, float epsilon = 0.0001F) const {
        const auto close = [epsilon](float left, float right) {
            return std::abs(left - right) <= epsilon;
        };
        return close(position.x, other.position.x) && close(position.y, other.position.y) &&
               close(position.z, other.position.z) &&
               close(rotationDegrees.x, other.rotationDegrees.x) &&
               close(rotationDegrees.y, other.rotationDegrees.y) &&
               close(rotationDegrees.z, other.rotationDegrees.z) &&
               close(scale.x, other.scale.x) && close(scale.y, other.scale.y) &&
               close(scale.z, other.scale.z);
    }
};

} // namespace lrender
