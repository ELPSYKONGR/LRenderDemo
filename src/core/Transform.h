/**
 * @file Editable object transform shared by scene, UI, and commands.
 * @author Codex
 * @created 2026-08-20
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>

namespace lrender
{

struct Transform
{
    DirectX::SimpleMath::Vector3 position = {0.0F, 0.0F, 0.0F};
    DirectX::SimpleMath::Vector3 rotationDegrees = {0.0F, 0.0F, 0.0F};
    DirectX::SimpleMath::Vector3 scale = {1.0F, 1.0F, 1.0F};

    /** Builds a row-major world matrix suitable for DirectXMath and ImGuizmo. */
    [[nodiscard]] DirectX::SimpleMath::Matrix ToMatrix() const;
    [[nodiscard]] bool NearlyEquals(const Transform& other, float epsilon = 0.0001F) const;
};

} // namespace lrender
